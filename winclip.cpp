// CopyHtml() - Copies given HTML to the clipboard.
// The HTML/BODY blanket is provided, so you only need to
// call it like CallHtml("<b>This is a test</b>");
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string.h>
#include <iostream>
#include <cstdio>

#include <cmark.h>

std::string markdownToHtml(const std::string& markdown) {
    char* html = cmark_markdown_to_html(markdown.c_str(), markdown.length(), CMARK_OPT_DEFAULT);
    std::string result(html);
    free(html);
    return result;
}

std::string GetClipboardText()
{
    std::string clipboardText = "";

    // Open the clipboard
    if (!OpenClipboard(nullptr))
    {
        std::cerr << "Failed to open clipboard" << std::endl;
        return clipboardText;
    }

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr)
    {
        std::cerr << "Failed to get clipboard data" << std::endl;
        CloseClipboard();
        return clipboardText;
    }

    // Lock the handle to get the actual text pointer
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr)
    {
        std::cerr << "Failed to lock clipboard data" << std::endl;
    }
    else
    {
        // Save text in a string class instance
        clipboardText = pszText;

        // Release the lock
        GlobalUnlock(hData);
    }

    // Release the clipboard
    CloseClipboard();

    return clipboardText;
}

void CopyHTML(const char* html)
{
    // Create temporary buffer for HTML header...
    char* buf = new char[400 + strlen(html)];
    if (!buf) return;

    // Get clipboard id for HTML format...
    static int cfid = 0;
    if (!cfid) cfid = RegisterClipboardFormat(L"HTML Format");

    // Create a template string for the HTML header...
    strcpy(buf,
        "Version:0.9\r\n"
        "StartHTML:00000000\r\n"
        "EndHTML:00000000\r\n"
        "StartFragment:00000000\r\n"
        "EndFragment:00000000\r\n"
        "<html><body>\r\n"
        "<!--StartFragment -->\r\n");

    // Append the HTML...
    strcat(buf, html);
    strcat(buf, "\r\n");
    // Finish up the HTML format...
    strcat(buf,
        "<!--EndFragment-->\r\n"
        "</body>\r\n"
        "</html>");

    // Now go back, calculate all the lengths, and write out the
    // necessary header information. Note, wsprintf() truncates the
    // string when you overwrite it so you follow up with code to replace
    // the 0 appended at the end with a '\r'...
    char* ptr = strstr(buf, "StartHTML");
    sprintf(ptr + 10, "%08u", strstr(buf, "<html>") - buf);
    *(ptr + 10 + 8) = '\r';

    ptr = strstr(buf, "EndHTML");
    sprintf(ptr + 8, "%08u", strlen(buf));
    *(ptr + 8 + 8) = '\r';

    ptr = strstr(buf, "StartFragment");
    sprintf(ptr + 14, "%08u", strstr(buf, "<!--StartFrag") - buf);
    *(ptr + 14 + 8) = '\r';

    ptr = strstr(buf, "EndFragment");
    sprintf(ptr + 12, "%08u", strstr(buf, "<!--EndFrag") - buf);
    *(ptr + 12 + 8) = '\r';

    // Now you have everything in place ready to put on the clipboard.
    // Open the clipboard...
    if (OpenClipboard(0))
    {
        // Empty what's in there...
        EmptyClipboard();

        // Allocate global memory for transfer...
        HGLOBAL hText = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(buf) + 4);

        // Put your string in the global memory...
        char* ptr = (char*)GlobalLock(hText);
        strcpy(ptr, buf);
        GlobalUnlock(hText);

        ::SetClipboardData(cfid, hText);

        CloseClipboard();
        // Free memory...
        GlobalFree(hText);
    }
    // Clean up...
    delete[] buf;
}

int main() {
    std::string clipboardContent = GetClipboardText();

    if (!clipboardContent.empty())
    {
        std::cout << "Clipboard content:" << std::endl;
        std::cout << clipboardContent << std::endl;
    }
    else
    {
        std::cout << "Clipboard is empty or contains non-text data." << std::endl;
    }

    std::string html = markdownToHtml(clipboardContent);
	const char* newContent = html.c_str();

	CopyHTML(newContent);
	return 0;
}