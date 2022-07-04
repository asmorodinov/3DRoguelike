#pragma once

void _check_gl_error(const char* file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define check_gl_error() _check_gl_error(__FILE__, __LINE__)

void CheckOpenGLError(const char* stmt, const char* fname, int line);

#ifdef GL_DEBUG
#define GL_CHECK(stmt)                               \
    do {                                             \
        stmt;                                        \
        CheckOpenGLError(#stmt, __FILE__, __LINE__); \
    } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

void CheckAssert(const char* stmt, const char* fname, int line);


#ifdef ENABLE_M_ASSERT
#define M_ASSERT(stmt) \
    if (!(stmt)) CheckAssert(#stmt, __FILE__, __LINE__);
#else
#define M_ASSERT(stmt) stmt
#endif
