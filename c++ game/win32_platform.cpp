#include <windows.h>
#include "utilities.cpp"

static bool running = true;

struct Render_State {
    int height, width;
    void* memory;
     BITMAPINFO bitmapinfo;
};
static Render_State render_state;

#include "renderer.cpp"
#include "platform_common.cpp"
#include "game.cpp"

LRESULT Wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    int size;
    switch (uMsg) {
    case WM_CLOSE:
    case WM_DESTROY:
        running = false;
        break;
    case WM_SIZE:
        RECT rect;
        GetClientRect(hwnd, &rect);
        render_state.width = rect.right - rect.left;
        render_state.height = rect.bottom - rect.top;
        size = render_state.width * render_state.height * sizeof(unsigned int);
        if (render_state.memory) {
            VirtualFree(render_state.memory, 0, MEM_RELEASE);
        }
        render_state.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        render_state.bitmapinfo.bmiHeader.biSize = sizeof(render_state.bitmapinfo.bmiHeader);
        render_state.bitmapinfo.bmiHeader.biWidth = render_state.width;
        render_state.bitmapinfo.bmiHeader.biHeight = render_state.height;
        render_state.bitmapinfo.bmiHeader.biPlanes = 1;
        render_state.bitmapinfo.bmiHeader.biBitCount = 32;
        render_state.bitmapinfo.bmiHeader.biCompression = BI_RGB;
        break;

    default:
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    
    ShowCursor(FALSE);

    WNDCLASS wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = "Game Window Class";
    wc.lpfnWndProc = Wndproc;
    RegisterClass(&wc);
    
    HWND window = CreateWindow(wc.lpszClassName, "Pong Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, hInstance, 0);
    
    
    {
        //Fullscreen
        SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
        SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    

    HDC hdc = GetDC(window);
    Input input = {};

    float delta_time = 0.01666666f; 
    LARGE_INTEGER frame_begin_time;
    QueryPerformanceCounter(&frame_begin_time);

    float performance_frequency;
    {
        LARGE_INTEGER perf;
        QueryPerformanceFrequency(&perf);
        performance_frequency = (float)perf.QuadPart;
    }

    while (running) {
        MSG message;
        
        for (int i = 0; i < BUTTON_COUNT; i++) {
            input.buttons[i].changed = false;
        }

        while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
            bool is_down;
            u32 vk_code;

            switch (message.message) {
                case WM_KEYUP:
                case WM_KEYDOWN:
                    vk_code = (u32)message.wParam;
                    is_down = ((message.lParam & (1 << 31)) == 0);

#define process_button(b, vk)\
case vk: \
input.buttons[b].changed = is_down != input.buttons[b].is_down;\
input.buttons[b].is_down = is_down;\
break;
                    switch (vk_code) {
                        process_button(BUTTON_UP, VK_UP);
                        process_button(BUTTON_DOWN, VK_DOWN);
                        process_button(BUTTON_W, 'W');
                        process_button(BUTTON_S, 'S');
                        process_button(BUTTON_LEFT, VK_LEFT);
                        process_button(BUTTON_RIGHT, VK_RIGHT);
                        process_button(BUTTON_ENTER, VK_RETURN);
                    }

                    break;
                default:
                    TranslateMessage(&message);
                    DispatchMessage(&message);
            }
        }

        //render_background();

        simulate_game(&input, delta_time);
        
        StretchDIBits(hdc, 0, 0, render_state.width, render_state.height, 0, 0, render_state.width, render_state.height, render_state.memory, &render_state.bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
        

        LARGE_INTEGER frame_end_time;
        QueryPerformanceCounter(&frame_end_time);
        delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency; 
        frame_begin_time = frame_end_time;

    
    }
}