#ifndef PYTHON_INTEGRATION_H
#define PYTHON_INTEGRATION_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool InitializePython(void);
void ExecutePythonScript(const char* scriptName);
void FinalizePython(void);

#ifdef __cplusplus
}
#endif

#endif // PYTHON_INTEGRATION_H
