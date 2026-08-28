#include "TestLib.h"

#include <stdio.h>
#include <wchar.h>

#include <chrono>
#include <cmath>
#include <thread>

extern "C" {
_OpStack libFunTest(SharedLibFunArg args) {
    fwprintf(stdout, L"========== [HxHLanguage Native Core Loaded] ==========\n");
    fwprintf(stdout, L"[INFO] Invoking C++ Shared Engine via CFFI...\n");
    fwprintf(stdout, L"[INFO] Initializing High-Precision Fractal Render Task...\n\n");

    auto startTime = std::chrono::high_resolution_clock::now();

    // 制造耗时的曼德博分形渲染
    const int width = 90;
    const int height = 30;
    const int max_iter = 16384;  // 故意提高迭代次数制造耗时

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double cr = (x - width / 1.5) * 4.0 / width;
            double ci = (y - height / 2.0) * 4.0 / height;
            double zr = 0.0, zi = 0.0;
            int iter = 0;

            while (zr * zr + zi * zi <= 4.0 && iter < max_iter) {
                double temp = zr * zr - zi * zi + cr;
                zi = 2.0 * zr * zi + ci;
                zr = temp;
                iter++;
            }

            wchar_t chars[] = L" .:-=+#%@";
            wchar_t c = chars[iter % 9];
            fwprintf(stdout, L"%c", c);
            fflush(stdout);
        }
        fwprintf(stdout, L"\n");
        // std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    fwprintf(stdout, L"\n[SUCCESS] Computation finished in %.2f ms.\n", elapsed);
    fwprintf(stdout, L"======================================================\n\n");

    _OpStack retVal;
    retVal.type = TYPE_INT;
    return retVal;
}
}