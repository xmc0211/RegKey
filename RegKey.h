// MIT License
//
// Copyright (c) 2025 RegKey - xmc0211 <xmc0211@qq.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef REGKEY_H
#define REGKEY_H

#include <windows.h>
#include <vector>
#include <string>
#include <sddl.h>
#include <aclapi.h>
#include <tchar.h>

// Default value name (empty string)
#define REG_DEFAULTVALUE ("")

// Default security descriptor
#define REGSECURITY_LOCK ("O:BAG:BAD:(A;;GR;;;WD)")
#define REGSECURITY_UNLOCK ("O:BAG:BAD:(A;;FA;;;WD)")

// Error
#define REG_SUCCESS ((HRESULT)0x0l)
#define REG_UNKNOWN_ERROR ((HRESULT)-0x1l)

#define REG_INVAILD_ROOT ((HRESULT)-0x2l)
#define REG_INVAILD_PATH ((HRESULT)-0x3l)
#define REG_INVAILD_POINTER ((HRESULT)-0x4l)
#define REG_ACCESS_DENIED ((HRESULT)-0x5l)
#define REG_INVAILD_VALUE ((HRESULT)-0x6l)
#define REG_INCORRECT_TYPE ((HRESULT)-0x7l)
#define REG_PATH_NOT_EXIST ((HRESULT)-0x8l)
#define REG_KEY_NOT_OPENED ((HRESULT)-0x9l)
#define REG_STR_TOO_LONG ((HRESULT)-0xAl)
#define REG_VALUE_NOT_EXIST ((HRESULT)-0xBl)
#define REG_BUFFER_OVERFLOW ((HRESULT)-0xCl)
#define REG_KEY_IS_ROOT ((HRESULT)-0xDl)

// Registry key class declaration
class REGKEY;

typedef ULONG64 QWORD; // QWORD is a 64 bit integer
typedef void (*REG_KEY_CALLBACK)(const REGKEY* pParent, LPCSTR lpName);
typedef void (*REG_VALUE_CALLBACK)(const REGKEY* pParent, LPCSTR lpName, DWORD dwType);

// Auxiliary function
BYTE HexCharToByte(CHAR cHex);
std::vector<BYTE> HexStringToByteArray(LPCSTR lpHex);

// Root term HKEY and std::string conversion
HKEY StringToHKEY(std::string lpStr);
std::string HKEYToString(HKEY hKey);

// WORD value type and std::string conversion
DWORD StringToType(std::string lpStr);
std::string TypeToString(DWORD dwType);

// Registry key class
class REGKEY {
private:
	HKEY hKey; // Main clause handle
	HKEY hRootKey; // Root term
	std::string cPath; // Path
	REGSAM ulSam; // Authority

public:
	REGKEY(); // Constructor function
	REGKEY(HKEY hRoot, LPCSTR lpPath, REGSAM ulSam, BOOL bCreateIfNotExist);
	REGKEY(const REGKEY& rOther); // Copy constructor function
	REGKEY& operator=(const REGKEY& rOther);
	~REGKEY(); // Destructor function

	// Create (Create if it does not exist when opened)
	HRESULT Create(HKEY hRoot, LPCSTR lpPath, REGSAM ulSam);
	// Open (Return error if it does not exist when opened)
	HRESULT Open(HKEY hRoot, LPCSTR lpPath, REGSAM ulSam);
	// Whether it was successfully opened or created
	BOOL Opened() const;
	// Close
	HRESULT Close();

	// Get root item
	HRESULT GetRootKey(HKEY* phOutKey) const;
	// Get path
	HRESULT GetPath(std::string* lpOutPath) const;
	// Get Authority
	HRESULT GetSam(REGSAM* pulOutSam) const;
	// Get parent item
	HRESULT GetParent(REGKEY* pFather, REGSAM hInSam) const;
	// Get sub item
	HRESULT GetSon(LPCSTR lpName, REGKEY* pSon, REGSAM hInSam) const;

	// Write REG_SZ value
	HRESULT WriteREGSZ(LPCSTR lpName, LPCSTR lpVal) const;
	// Write REG_EXPAND_SZ value
	HRESULT WriteREGEXPANDSZ(LPCSTR lpName, LPCSTR lpVal) const;
	// Write REG_DWORD value
	HRESULT WriteREGDWORD(LPCSTR lpName, DWORD dwVal) const;
	// Write REG_QWORD value
	HRESULT WriteREGQWORD(LPCSTR lpName, QWORD ullVal) const;
	// Write REG_BINARY value
	// The data is represented as a hexadecimal string (such as "EB589033907C"). Invalid characters will be converted to 0.
	// If the number of characters is not even, discard the last character.
	HRESULT WriteREGBINARY(LPCSTR lpName, LPCSTR lpVal) const;
	// Write REG_MULTI_SZ value
	HRESULT WriteREGMULTISZ(LPCSTR lpName, std::vector<LPCSTR> lpVal) const;

	// Delete registry values
	HRESULT DeleteValue(LPCSTR lpName) const;
	// Delete the open registry key and close
	HRESULT Delete();

	// Get value type
	// The pointer in this function can be empty, indicating that the corresponding value is not obtained.
	HRESULT GetTypeSize(LPCSTR lpName, DWORD* pdwType, DWORD* pdwSize) const;
	// Read REG_SZ value
	HRESULT ReadREGSZ(LPCSTR lpName, std::string* lpRes) const;
	// Read REG_EXPAND_SZ value
	HRESULT ReadREGEXPANDSZ(LPCSTR lpName, std::string* lpRes) const;
	// Read REG_DWORD value
	HRESULT ReadREGDWORD(LPCSTR lpName, DWORD* dwRes) const;
	// Read REG_QWORD value
	HRESULT ReadREGQWORD(LPCSTR lpName, QWORD* qwRes) const;
	// Read REG_BINARY value
	HRESULT ReadREGBINARY(LPCSTR lpName, std::string* lpRes) const;
	// Read REG_MULTI_SZ value
	HRESULT ReadREGMULTISZ(LPCSTR lpName, std::vector<std::string>* lpRes) const;

	// Enum the first level sub values under opened item
	HRESULT EnumValue(REG_VALUE_CALLBACK callback) const;
	// Enum the first level sub items under opened item
	HRESULT EnumKey(REG_KEY_CALLBACK callback) const;
	// Enum all sub values under opened item
	HRESULT EnumAllValue(REG_VALUE_CALLBACK callback) const;
	// Enum all sub items under opened item
	HRESULT EnumAllKey(REG_KEY_CALLBACK callback) const;

	// Set registry key permissions
	// Provide security descriptor string.
	HRESULT SetSecurityInfo(LPCSTR lpSddl) const;

};

#endif
