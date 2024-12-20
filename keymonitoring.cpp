#include <iostream>
#include <windows.h>
#include <string>

// Global variables
std::string lastWord;
std::string currentWord;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            DWORD vkCode = pKeyboard->vkCode;
            char keyChar = MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);

            if (vkCode == VK_TAB) {
                // Store the last word when Tab is pressed
                lastWord = currentWord;
                currentWord.clear();
                std::cout << "Last word: " << lastWord << std::endl;
            } else {
                if (isalnum(keyChar)) {
                    currentWord += keyChar;
                } else if (isspace(keyChar)) {
                    lastWord = currentWord;
                    currentWord.clear();
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hhkLowLevelKybd);
    return 0;
}
