// #define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

// CopyHtml() - Copies given HTML to the clipboard.
// The HTML/BODY blanket is provided, so you only need to
// call it like CallHtml("<b>This is a test</b>");

void CopyHTML(const char* html)
{
    // Create temporary buffer for HTML header...
    size_t html_len = strlen(html);
    char* buf = new char[400 + html_len];
    if (!buf) return;

    // Get clipboard id for HTML format...
    static int cfid = 0;
    if (!cfid) cfid = RegisterClipboardFormatW(L"HTML Format");

    // Create a template string for the HTML header...
    snprintf(buf, 00 + html_len,
        "Version:0.9\r\n"
        "StartHTML:00000000\r\n"
        "EndHTML:00000000\r\n"
        "StartFragment:00000000\r\n"
        "EndFragment:00000000\r\n"
        "<html><body>\r\n"
        "<!--StartFragment -->\r\n"
        "%s\r\n"
        "<!--EndFragment-->\r\n"
        "</body>\r\n"
        "</html>",
        html);

    // Now go back, calculate all the lengths, and write out the
    // necessary header information.
    char* ptr = strstr(buf, "StartHTML");
    if (ptr != nullptr) {
        snprintf(ptr + 10, 9, "%08u", static_cast<unsigned>(strstr(buf, "<html>") - buf));
        *(ptr + 10 + 8) = '\r';
    }

    ptr = strstr(buf, "EndHTML");
    if (ptr != nullptr) {
        snprintf(ptr + 8, 9, "%08u", static_cast<unsigned>(strlen(buf)));
        *(ptr + 8 + 8) = '\r';
    }

    ptr = strstr(buf, "StartFragment");
    if (ptr != nullptr) {
        snprintf(ptr + 14, 9, "%08u", static_cast<unsigned>(strstr(buf, "<!--StartFrag") - buf));
        *(ptr + 14 + 8) = '\r';
    }
    ptr = strstr(buf, "EndFragment");
    if (ptr != nullptr) {
        snprintf(ptr + 12, 9, "%08u", static_cast<unsigned>(strstr(buf, "<!--EndFrag") - buf));
        *(ptr + 12 + 8) = '\r';
    }

    // Now you have everything in place ready to put on the clipboard.
    // Open the clipboard...
    if (OpenClipboard(0))
    {
        // Empty what's in there...
        EmptyClipboard();

        // Allocate global memory for transfer...
        HGLOBAL hText = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(buf) + 4);
        if (hText) {
            // Put your string in the global memory...
            char* pchr = (char*)GlobalLock(hText);
            if (pchr) {
                strcpy(pchr, buf);
                GlobalUnlock(hText);
                ::SetClipboardData(cfid, hText);

                strcpy(pchr, buf);
                GlobalUnlock(hText);

                ::SetClipboardData(cfid, hText);

                CloseClipboard();
                // Free memory...
                GlobalFree(hText);
            }
        }
    }
    // Clean up...
    delete[] buf;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    file.close();
    return 0;
}
