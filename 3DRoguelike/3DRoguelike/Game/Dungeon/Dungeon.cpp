#include "Dungeon.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <vector>
#include <unordered_set>

#include "../Algorithms/Pathfind.h"
#include "../Algorithms/Delaunay3D.h"
#include "../Algorithms/MST.h"

#include "../Assets.h"

static const auto offset = glm::ivec3(5, 5, 5);

Dungeon::Dungeon(const Dimensions& dimensions_, SeedType seed_)
    : dimensions(FromIVec3(AsIVec3(dimensions_) + 2 * offset)), seed(seed_), rng(seed), tiles(dimensions, Tile()), rooms(), renderer(), spawn() {
}

void Dungeon::SetSeed(SeedType seed_) {
    seed = seed_;
    rng.Seed(seed);
}

SeedType Dungeon::GetSeed() const {
    return seed;
}

// only one room type for now
Room getRandomRoom(RNG& rng) {
    if (rng.RandomBool(0.33f)) {
        return std::make_shared<OvalRoom>();
    } else if (rng.RandomBool(0.5f)) {
        return std::make_shared<EllipsoidRoom>();
    } else {
        return std::make_shared<RectRoom>();
    }
}

void Dungeon::placeRooms() {
    auto tries = 1000;
    auto roomCnt = 10;

    do {
        auto newRoom = getRandomRoom(rng);
        auto roomSeed = rng.IntUniform<SeedType>(SeedType(0), SeedType(-1));
        auto newSeed = rng.IntUniform<SeedType>(SeedType(0), SeedType(-1));

        SetSeed(roomSeed);
        newRoom->Generate(rng, roomSeed);
        newRoom->offset = rng.RandomIVec3(offset, AsIVec3(dimensions) - AsIVec3(newRoom->size) - offset);
        SetSeed(newSeed);

        if (!BoxFitsIntoBox(Box{newRoom->offset, newRoom->size}, Box{glm::ivec3(), dimensions})) {
            continue;
        }

        auto intersect = false;
        for (const auto& room : rooms) {
            if (RoomsIntersect(room, newRoom)) {
                intersect = true;
                break;
            }
        }
        if (intersect) {
            continue;
        }

        --roomCnt;
        newRoom->Place(tiles);
        rooms.push_back(newRoom);
    } while (--tries > 0 && roomCnt != 0);
}

void Dungeon::placeCorridors() {
    auto air = Tile{TileType::CorridorAir, TileOrientation::None, TextureType::None, glm::vec3(1.0f)};

    // determine which rooms should be connected

    auto points = std::vector<glm::vec3>();
    for (const auto& room : rooms) {
        points.push_back(RoomCenter(room));
    }

    // calculate triangulation
    auto edges = Delaunay3D(points);

    // calculate MST
    auto weights = std::vector<Weight>();
    for (const auto& edge : edges) {
        auto diff = points[edge.v1] - points[edge.v2];
        diff.y *= 10.0f;  // make moving upwards more expensive
        weights.push_back(Weight(glm::length2(diff)));
    }
    auto mstEdges = MinimumSpanningTree(edges, points.size(), weights);

    // add some edges from triangulation to MST edges
    auto finalEdges = std::unordered_set<Edge, Edge::HashFunction>(mstEdges.begin(), mstEdges.end());
    for (const auto& edge : edges) {
        if (rng.RandomBool(0.2f)) {
            finalEdges.insert(edge);
        }
    }

    // connect rooms with corridors
    auto pathfinder = Pathfinder(dimensions);

    std::cout << "Connecting rooms..." << std::endl;
    for (const auto& [v1, v2] : finalEdges) {
        std::cout << v1 << " " << v2 << std::endl;

        const auto& r1 = rooms[v1];
        const auto& r2 = rooms[v2];

        auto wall = Tile{
            TileType::Block, TileOrientation::None, TextureType::Texture2,
            glm::vec3(0.4f, 0.3f, 0.8f) + 0.2f * glm::vec3(rng.RealUniform(-1.0f, 1.0f), rng.RealUniform(-1.0f, 1.0f), rng.RealUniform(-1.0f, 1.0f))};
        auto stairs = Tile{
            TileType::StairsAir, TileOrientation::None, TextureType::Texture2,
            glm::vec3(0.4f, 0.3f, 0.8f) + 0.2f * glm::vec3(rng.RealUniform(-1.0f, 1.0f), rng.RealUniform(-1.0f, 1.0f), rng.RealUniform(-1.0f, 1.0f))};

        auto startTiles = r1->GetEdgeTiles();
        auto finishTiles = r2->GetEdgeTiles();
        for (auto& tile : startTiles) {
            tile = tile + r1->offset;
        }
        for (auto& tile : finishTiles) {
            tile = tile + r2->offset;
        }

        auto path = pathfinder.FindPath(startTiles, finishTiles, RoomCenterCoords(r2), tiles);
        if (!path.empty()) {
            PlacePathWithStairs(path, tiles, wall, air, stairs);
        } else {
            LOG_ASSERT(false);
        }
    }
}

void Dungeon::reset() {
    rooms.clear();
    tiles = TilesVec(dimensions, Tile());
}

void addTile(const glm::ivec3& coords, std::vector<PositionColor>& blocks, std::array<std::vector<PositionColor>, 4>& stairs, const TilesVec& tiles) {
    const auto& tile = tiles.GetInOrOutOfBounds(coords);
    if (IsSolidBlock(tile.type)) {
        blocks.push_back({glm::vec3(coords), tile.color, 1.0f});
    } else if (tile.type == TileType::StairsTopPart && tile.orientation == TileOrientation::North) {
        stairs[0].push_back({glm::vec3(coords), tile.color, 1.0f});
    } else if (tile.type == TileType::StairsTopPart && tile.orientation == TileOrientation::West) {
        stairs[1].push_back({glm::vec3(coords), tile.color, 1.0f});
    } else if (tile.type == TileType::StairsTopPart && tile.orientation == TileOrientation::South) {
        stairs[2].push_back({glm::vec3(coords), tile.color, 1.0f});
    } else if (tile.type == TileType::StairsTopPart && tile.orientation == TileOrientation::East) {
        stairs[3].push_back({glm::vec3(coords), tile.color, 1.0f});
    }
}

void Dungeon::Generate() {
    reset();

    std::cout << "Seed: " << seed << std::endl;

    placeRooms();
    placeCorridors();

    auto roomIndex = rng.IntUniform<size_t>(0, rooms.size() - 1);
    if (Assets::HasConfigParameter("starting-room")) {
        roomIndex = Assets::GetConfigParameter<size_t>("starting-room");
    }

    const auto& room = rooms[roomIndex];
    spawn = RoomCenterCoords(room);

    auto tilesData = std::vector<PositionColor>();
    auto stairsData = std::array<std::vector<PositionColor>, 4>({{}, {}, {}, {}});

    for (size_t x = 0; x < dimensions.width; ++x) {
        for (size_t y = 0; y < dimensions.height; ++y) {
            for (size_t z = 0; z < dimensions.length; ++z) {
                auto coords = glm::ivec3{x, y, z};
                addTile(coords, tilesData, stairsData, tiles);
            }
        }
    }
    for (const auto& [coords, tile] : tiles.GetOutOfBoundsMap()) {
        addTile(coords, tilesData, stairsData, tiles);
    }

    renderer.InitInstancedRendering(tilesData, stairsData);
}

void Dungeon::Render() {
    renderer.RenderTilesInstanced();
}

const TilesVec& Dungeon::GetTiles() const {
    return tiles;
}

glm::ivec3 Dungeon::GetSpawnPoint() const {
    return spawn;
}

size_t Dungeon::WhichRoomPointIsInside(const glm::ivec3& coords) const {
    for (size_t i = 0; i < rooms.size(); ++i) {
        if (PointInsideBox(coords, Box{rooms[i]->offset, rooms[i]->size})) {
            return i;
        }
    }

    return rooms.size();
}
