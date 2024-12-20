#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <chrono>
#include <windows.h>

// Global variables
std::string lastWord;
std::string currentWord;

// Store text expansion pairs
std::map<std::string, std::string> expansions = {
    {"hru", "How are you?"},
    {"ty", "Thank you"},
    {"brb", "Be right back"}
};

// Function to simulate backspace to remove the shortcut text
void simulateBackspace(int count) {
    for(int i = 0; i < count; i++) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_BACK;
        SendInput(1, &input, sizeof(INPUT));
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}

// Function to simulate key press for expansion
void simulateKeyPress(char key) {
    SHORT vk = VkKeyScanA(key);
    if (vk == -1) return; // Skip if the character is not mappable

    bool shiftNeeded = HIBYTE(vk) & 1; // Check if shift is required
    INPUT inputs[4] = {};
    int inputCount = 0;

    if (shiftNeeded) {
        inputs[inputCount].type = INPUT_KEYBOARD;
        inputs[inputCount].ki.wVk = VK_SHIFT;
        inputCount++;
    }

    // Key press
    inputs[inputCount].type = INPUT_KEYBOARD;
    inputs[inputCount].ki.wVk = LOBYTE(vk);
    inputCount++;

    // Key release
    inputs[inputCount].type = INPUT_KEYBOARD;
    inputs[inputCount].ki.wVk = LOBYTE(vk);
    inputs[inputCount].ki.dwFlags = KEYEVENTF_KEYUP;
    inputCount++;

    if (shiftNeeded) {
        inputs[inputCount].type = INPUT_KEYBOARD;
        inputs[inputCount].ki.wVk = VK_SHIFT;
        inputs[inputCount].ki.dwFlags = KEYEVENTF_KEYUP;
        inputCount++;
    }

    SendInput(inputCount, inputs, sizeof(INPUT));
}

// The keyboard hook callback function
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT *)lParam;
        
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            DWORD vkCode = pKeyboard->vkCode;
            char keyChar = MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);

            if (vkCode == VK_TAB) {
                // Check if current word is a shortcut
                auto it = expansions.find(currentWord);
                if (it != expansions.end()) {
                    // Remove the shortcut text
                    simulateBackspace(currentWord.length());
                    
                    // Type out the expansion
                    for(char c : it->second) {
                        simulateKeyPress(c);
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    
                    lastWord = currentWord;
                    currentWord.clear();
                    
                    // Return 1 to block the Tab key
                    return 1;
                }
            } else {
                if (isalnum(keyChar)) {
                    currentWord += tolower(keyChar);
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
    // Print available shortcuts
    std::cout << "Text expander started. Press Tab after typing a shortcut." << std::endl;
    
    // Set up the keyboard hook
    HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
    
    if (hhkLowLevelKybd == NULL) {
        std::cout << "Failed to set up keyboard hook!" << std::endl;
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    UnhookWindowsHookEx(hhkLowLevelKybd);
    return 0;
}