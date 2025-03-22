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

#include "RegKey.h"
#include <aclapi.h>
#include <tchar.h>

#define REG_VAILD_ROOTKEY(i) ((i) == HKEY_CLASSES_ROOT || (i) == HKEY_CURRENT_USER || (i) == HKEY_LOCAL_MACHINE || (i) == HKEY_USERS || (i) == HKEY_CURRENT_CONFIG)
#define REG_VAILD_PATH(i) ((i).size() <= MAX_PATH)
#define DEC(i) if (dwType == D##i) return #i;

BYTE HexCharToByte(CHAR cHex) {
	if (cHex >= '0' && cHex <= '9') return cHex - '0';
	if (cHex >= 'a' && cHex <= 'f') return cHex - 'a' + 10;
	if (cHex >= 'A' && cHex <= 'F') return cHex - 'A' + 10;
	else return 0;
}

std::vector<BYTE> HexStringToByteArray(LPCSTR lpHex) {
	std::vector<BYTE> bytes;
	size_t len = strlen(lpHex);
	for (size_t i = 0; i < len; i += 2) {
		BYTE highNibble = HexCharToByte(lpHex[i]);
		BYTE lowNibble = HexCharToByte(lpHex[i + 1]);
		BYTE bNow = (highNibble << 4) | lowNibble;
		bytes.push_back(bNow);
	}
	return bytes;
}


HKEY StringToHKEY(std::string lpStr) {
	if (lpStr == "HKEY_CLASSES_ROOT" || lpStr == "HKCR") return HKEY_CLASSES_ROOT;
	if (lpStr == "HKEY_CURRENT_USER" || lpStr == "HKCU") return HKEY_CURRENT_USER;
	if (lpStr == "HKEY_LOCAL_MACHINE" || lpStr == "HKLM") return HKEY_LOCAL_MACHINE;
	if (lpStr == "HKEY_USERS" || lpStr == "HKU") return HKEY_USERS;
	if (lpStr == "HKEY_CURRENT_CONFIG" || lpStr == "HKCC") return HKEY_CURRENT_CONFIG;
	return HKEY_LOCAL_MACHINE;
}
std::string HKEYToString(HKEY hKey) {
	if (hKey == HKEY_CLASSES_ROOT) return "HKEY_CLASSES_ROOT";
	if (hKey == HKEY_CURRENT_USER) return "HKEY_CURRENT_USER";
	if (hKey == HKEY_LOCAL_MACHINE) return "HKEY_LOCAL_MACHINE";
	if (hKey == HKEY_USERS) return "HKEY_USERS";
	if (hKey == HKEY_CURRENT_CONFIG) return "HKEY_CURRENT_CONFIG";
	return "HKEY_LOCAL_MACHINE";
}


DWORD StringToType(std::string lpStr) {
	if (lpStr == "REG_SZ") return REG_SZ;
	if (lpStr == "REG_EXPAND_SZ") return REG_EXPAND_SZ;
	if (lpStr == "REG_DWORD") return REG_DWORD;
	if (lpStr == "REG_QWORD") return REG_QWORD;
	if (lpStr == "REG_BINARY") return REG_BINARY;
	if (lpStr == "REG_MULTI_SZ") return REG_MULTI_SZ;
	return REG_SZ;
}
std::string TypeToString(DWORD dwType) {
	if (dwType == REG_SZ) return "REG_SZ";
	if (dwType == REG_EXPAND_SZ) return "REG_EXPAND_SZ";
	if (dwType == REG_DWORD) return "REG_DWORD";
	if (dwType == REG_QWORD) return "REG_QWORD";
	if (dwType == REG_BINARY) return "REG_BINARY";
	if (dwType == REG_MULTI_SZ) return "REG_MULTI_SZ";
	return "REG_SZ";
}


HRESULT REGKEY::Create(HKEY hInRootKey, LPCSTR lpInPath, REGSAM ulInSam) {
	if (!REG_VAILD_ROOTKEY(hInRootKey)) return REG_INVAILD_ROOT;
	if (hInRootKey == 0) return REG_INVAILD_ROOT;
	HRESULT hRes = RegCreateKeyExA(
		hInRootKey, 
		lpInPath, 
		0, 
		nullptr, 
		REG_OPTION_NON_VOLATILE, 
		ulInSam, 
		nullptr, 
		&hKey, 
		nullptr
	);
	if (hRes != ERROR_SUCCESS) {
		hKey = NULL;
		return REG_UNKNOWN_ERROR;
	}
	hRootKey = hInRootKey;
	cPath = lpInPath;
	ulSam = ulInSam;
	return REG_SUCCESS;
}

HRESULT REGKEY::Open(HKEY hInRootKey, LPCSTR lpInPath, REGSAM ulInSam) {
	if (!REG_VAILD_ROOTKEY(hInRootKey)) return REG_INVAILD_ROOT;
	if (hInRootKey == 0) return REG_INVAILD_ROOT;
	HRESULT hRes = RegOpenKeyExA(
		hInRootKey, 
		lpInPath, 
		0, 
		ulInSam, 
		&hKey
	);
	if (hRes != ERROR_SUCCESS) {
		hKey = NULL;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_PATH_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	hRootKey = hInRootKey;
	cPath = lpInPath;
	ulSam = ulInSam;
	return REG_SUCCESS;
}

BOOL REGKEY::Opened() const {
	BOOL bRes = (hKey != NULL);
	return bRes;
}

HRESULT REGKEY::Close() {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	HRESULT hRes = RegCloseKey(hKey);
	hKey = NULL;
	if (hRes != ERROR_SUCCESS) return REG_UNKNOWN_ERROR;
	hRootKey = NULL;
	cPath = "";
	ulSam = 0;
	return REG_SUCCESS;
}


HRESULT REGKEY::GetRootKey(HKEY* phOutKey) const {
	if (phOutKey == nullptr) return REG_INVAILD_POINTER;
	*phOutKey = hRootKey;
	return REG_SUCCESS;
}

HRESULT REGKEY::GetPath(std::string* lpOutPath) const {
	if (lpOutPath == nullptr) return REG_INVAILD_POINTER;
	*lpOutPath = cPath;
	return REG_SUCCESS;
}

HRESULT REGKEY::GetSam(REGSAM* pulOutSam) const {
	if (pulOutSam == nullptr) return REG_INVAILD_POINTER;
	*pulOutSam = ulSam;
	return REG_SUCCESS;
}

HRESULT REGKEY::GetParent(REGKEY* pFather, REGSAM hInSam) const {
	REGKEY rFather;
	size_t LastKey = cPath.find_last_of("\\");
	if (LastKey == std::string::npos) return REG_KEY_IS_ROOT;
	std::string NewPath = cPath.substr(0, LastKey);
	if (pFather == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = rFather.Open(hRootKey, NewPath.c_str(), hInSam);
	if (hRes != REG_SUCCESS) return hRes;
	*pFather = REGKEY(rFather);
	return REG_SUCCESS;
}

HRESULT REGKEY::GetSon(LPCSTR lpName, REGKEY* pSon, REGSAM hInSam) const {
	REGKEY rSon;
	std::string NewPath = cPath + "\\" + lpName;
	if (pSon == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = rSon.Open(hRootKey, NewPath.c_str(), hInSam);
	if (hRes != REG_SUCCESS) return hRes;
	*pSon = REGKEY(rSon);
	return REG_SUCCESS;
}


REGKEY::REGKEY() : hKey(nullptr), hRootKey(nullptr), ulSam(0), cPath("") {
	return;
}
REGKEY::REGKEY(HKEY hInRootKey, LPCSTR lpInPath, REGSAM ulInSam, BOOL bCreateIfNotExist) : hKey(nullptr), hRootKey(nullptr), ulSam(0), cPath("") {
	if (!REG_VAILD_PATH(std::string(lpInPath))) return;
	HRESULT hRes = 0;
	if (bCreateIfNotExist) hRes = Create(hInRootKey, lpInPath, ulInSam);
	else hRes = Open(hInRootKey, lpInPath, ulInSam);
	if (hRes == REG_SUCCESS) {
		ulSam = ulInSam;
		hRootKey = hInRootKey;
		cPath = lpInPath;
	}
}
REGKEY::REGKEY(const REGKEY& rOther) : hKey(nullptr), hRootKey(nullptr), ulSam(0), cPath("") {
	if (!rOther.Opened()) return;
	if (!REG_VAILD_PATH(rOther.cPath)) return;
	if (Open(rOther.hRootKey, rOther.cPath.c_str(), rOther.ulSam) == REG_SUCCESS) {
		ulSam = rOther.ulSam;
		hRootKey = rOther.hRootKey;
		cPath = rOther.cPath;
	}
}
REGKEY& REGKEY::operator=(const REGKEY& rOther) {
	if (!rOther.Opened()) return *this;
	if (!REG_VAILD_PATH(rOther.cPath)) return *this;
	if (Opened()) Close();
	if (Open(rOther.hRootKey, rOther.cPath.c_str(), rOther.ulSam) == REG_SUCCESS) {
		ulSam = rOther.ulSam;
		hRootKey = rOther.hRootKey;
		cPath = rOther.cPath;
	}
	return *this;
}
REGKEY::~REGKEY() {
	if (Opened()) Close();
}

HRESULT REGKEY::WriteREGSZ(LPCSTR lpName, LPCSTR lpVal) const {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpVal == nullptr) return REG_INVAILD_VALUE;
	if (strlen(lpVal) > 0xff) return REG_STR_TOO_LONG;
	HRESULT hRes = RegSetValueExA(
		hKey, 
		lpName, 
		0, 
		REG_SZ, 
		reinterpret_cast<const BYTE*>(lpVal), 
		(DWORD)(strlen(lpVal) + 1) * sizeof(CHAR)
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}

HRESULT REGKEY::WriteREGEXPANDSZ(LPCSTR lpName, LPCSTR lpVal) const {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpVal == nullptr) return REG_INVAILD_VALUE;
	if (strlen(lpVal) > 0xff) return REG_STR_TOO_LONG;
	HRESULT hRes = RegSetValueExA(
		hKey,
		lpName,
		0,
		REG_EXPAND_SZ,
		reinterpret_cast<const BYTE*>(lpVal),
		(DWORD)(strlen(lpVal) + 1) * sizeof(CHAR)
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}

HRESULT REGKEY::WriteREGDWORD(LPCSTR lpName, DWORD dwVal) const {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	HRESULT hRes = RegSetValueExA(
		hKey,
		lpName,
		0,
		REG_DWORD,
		reinterpret_cast<const BYTE*>(&dwVal),
		sizeof(DWORD)
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}
HRESULT REGKEY::WriteREGQWORD(LPCSTR lpName, QWORD ullVal) const {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	HRESULT hRes = RegSetValueExA(
		hKey,
		lpName,
		0,
		REG_QWORD,
		reinterpret_cast<const BYTE*>(&ullVal),
		sizeof(QWORD)
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}
HRESULT REGKEY::WriteREGBINARY(LPCSTR lpName, LPCSTR lpVal) const {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpVal == nullptr) return REG_INVAILD_POINTER;
	std::vector<BYTE> bytes = HexStringToByteArray(lpVal);
	HRESULT hRes = RegSetValueExA(
		hKey,
		lpName,
		0,
		REG_BINARY,
		bytes.data(), 
		static_cast<DWORD>(bytes.size())
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}
HRESULT REGKEY::WriteREGMULTISZ(LPCSTR lpName, std::vector<LPCSTR> lpVal) const {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	std::vector<CHAR> lpData;
	for (const auto& itVal : lpVal) {
		size_t ulStrSize = strlen(itVal);
		for (INT pChar = 0; pChar < ulStrSize; pChar++) {
			lpData.push_back(itVal[pChar]);
		}
		lpData.push_back('\0');
	}
	lpData.push_back('\0');
	HRESULT hRes = RegSetValueExA(
		hKey, 
		lpName, 
		0, 
		REG_MULTI_SZ, 
		reinterpret_cast<const BYTE*>(lpData.data()), 
		static_cast<DWORD>(lpData.size())
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}

HRESULT REGKEY::DeleteValue(LPCSTR lpName) const {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpName == nullptr) return REG_INVAILD_VALUE;
	if (strlen(lpName) > 0xff) return REG_STR_TOO_LONG;
	HRESULT hRes = RegDeleteValueA(
		hKey, 
		lpName
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}
HRESULT REGKEY::Delete() {
	if (!Opened()) return REG_KEY_NOT_OPENED;
	HRESULT hRes = RegDeleteKeyExA(
		hKey, 
		"", 
		ulSam, 
		0
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_PATH_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	Close();
	return REG_SUCCESS;
}

HRESULT REGKEY::GetTypeSize(LPCSTR lpName, DWORD* pdwType, DWORD* pdwSize) const {
	DWORD dwType = 0;
	DWORD dwSize = 0;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	HRESULT hRes = RegQueryValueExA(
		hKey,
		lpName,
		nullptr,
		&dwType,
		nullptr,
		&dwSize
	);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	if (pdwType != nullptr) *pdwType = dwType;
	if (pdwSize != nullptr) *pdwSize = dwSize;
	return REG_SUCCESS;
}
HRESULT REGKEY::ReadREGSZ(LPCSTR lpName, std::string* lpRes) const {
	DWORD dwType = 0;
	DWORD dwSize = 0;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpRes == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = GetTypeSize(lpName, &dwType, &dwSize);
	if (hRes != REG_SUCCESS) return hRes;
	if (dwType != REG_SZ) return REG_INCORRECT_TYPE;
	DWORD dwBufferSize = dwSize / sizeof(CHAR) + 1;
	std::vector<CHAR> buffer(dwBufferSize);
	hRes = RegQueryValueExA(
		hKey,
		lpName,
		nullptr,
		&dwType,
		reinterpret_cast<LPBYTE>(buffer.data()),
		&dwSize
	);
	if (hRes != REG_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	buffer[dwBufferSize - 1] = '\0';
	*lpRes = buffer.data();
	return REG_SUCCESS;
}
HRESULT REGKEY::ReadREGEXPANDSZ(LPCSTR lpName, std::string* lpRes) const {
	DWORD dwType = 0;
	DWORD dwSize = 0;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpRes == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = GetTypeSize(lpName, &dwType, &dwSize);
	if (hRes != REG_SUCCESS) return hRes;
	if (dwType != REG_EXPAND_SZ) return REG_INCORRECT_TYPE;
	DWORD dwBufferSize = dwSize / sizeof(CHAR) + 1;
	std::vector<CHAR> buffer(dwBufferSize);
	hRes = RegQueryValueExA(
		hKey,
		lpName,
		nullptr,
		&dwType,
		reinterpret_cast<LPBYTE>(buffer.data()),
		&dwSize
	);
	if (hRes != REG_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	buffer[dwBufferSize - 1] = '\0';
	*lpRes = buffer.data();
	return REG_SUCCESS;
}
HRESULT REGKEY::ReadREGDWORD(LPCSTR lpName, DWORD* dwRes) const {
	DWORD dwType = 0;
	DWORD dwSize = 0;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (dwRes == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = GetTypeSize(lpName, &dwType, &dwSize);
	if (hRes != REG_SUCCESS) return hRes;
	if (dwType != REG_DWORD) return REG_INCORRECT_TYPE;
	DWORD dwValue = 0;
	hRes = RegQueryValueExA(
		hKey, 
		lpName, 
		nullptr, 
		&dwType, 
		reinterpret_cast<LPBYTE>(&dwValue), 
		&dwSize
	);
	if (hRes != REG_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	*dwRes = dwValue;
	return REG_SUCCESS;
}
HRESULT REGKEY::ReadREGQWORD(LPCSTR lpName, QWORD* qwRes) const {
	DWORD dwType = 0;
	DWORD dwSize = 0;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (qwRes == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = GetTypeSize(lpName, &dwType, &dwSize);
	if (hRes != REG_SUCCESS) return hRes;
	if (dwType != REG_QWORD) return REG_INCORRECT_TYPE;
	QWORD qwValue = 0;
	hRes = RegQueryValueExA(
		hKey,
		lpName,
		nullptr,
		&dwType,
		reinterpret_cast<LPBYTE>(&qwValue),
		&dwSize
	);
	if (hRes != REG_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	*qwRes = qwValue;
	return REG_SUCCESS;
}
HRESULT REGKEY::ReadREGBINARY(LPCSTR lpName, std::string* lpRes) const {
	DWORD dwType = 0;
	DWORD dwSize = 0;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpRes == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = GetTypeSize(lpName, &dwType, &dwSize);
	if (hRes != REG_SUCCESS) return hRes;
	if (dwType != REG_BINARY) return REG_INCORRECT_TYPE;
	std::vector<BYTE> buffer(dwSize);
	hRes = RegQueryValueExA(
		hKey,
		lpName,
		nullptr,
		&dwType,
		buffer.data(),
		&dwSize
	);
	if (hRes != REG_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	for (const BYTE& b : buffer) {
		CHAR hex[3];
		sprintf_s(hex, sizeof hex, "%02X", b);
		*lpRes += hex;
	}
	return REG_SUCCESS;
}
HRESULT REGKEY::ReadREGMULTISZ(LPCSTR lpName, std::vector<std::string>* lpRes) const {
	DWORD dwType = 0;
	DWORD dwSize = 0;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (lpRes == nullptr) return REG_INVAILD_POINTER;
	HRESULT hRes = GetTypeSize(lpName, &dwType, &dwSize);
	if (hRes != REG_SUCCESS) return hRes;
	std::vector<CHAR> buffer(1024);
	std::vector<std::string> Res;
	hRes = RegQueryValueExA(
		hKey, 
		lpName, 
		nullptr, 
		&dwType, 
		reinterpret_cast<LPBYTE>(buffer.data()), 
		&dwSize
	);
	if (hRes != REG_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		if (hRes == ERROR_FILE_NOT_FOUND) return REG_VALUE_NOT_EXIST;
		return REG_UNKNOWN_ERROR;
	}
	const CHAR* p = buffer.data();
	while (*p) {
		Res.push_back(p);
		p += Res.back().length() + 1;
	}
	*lpRes = Res;
	return REG_SUCCESS;
}

HRESULT REGKEY::EnumValue(REG_VALUE_CALLBACK callback) const {
	DWORD index = 0, vType = 0, vNameSize = 0;
	CHAR vName[256] = "";
	HRESULT hRes = REG_SUCCESS;
	if (!Opened()) return REG_KEY_NOT_OPENED;

	while (1) {
		vNameSize = 256;
		HRESULT lRes = RegEnumValueA(
			hKey, 
			index, 
			vName, 
			&vNameSize, 
			nullptr, 
			&vType, 
			nullptr,
			nullptr
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_NO_MORE_ITEMS) {
			if (lRes == ERROR_ACCESS_DENIED) hRes = REG_ACCESS_DENIED;
			else hRes = REG_UNKNOWN_ERROR;
		}
		if (lRes == ERROR_NO_MORE_ITEMS) break;
		vName[vNameSize] = '\0';
		callback(this, vName, vType);
		index++;
	}
	return hRes;
}
HRESULT REGKEY::EnumKey(REG_KEY_CALLBACK callback) const {
	DWORD index = 0, kNameSize = 0;
	CHAR kName[256] = "";
	HRESULT hRes = REG_SUCCESS;
	if (!Opened()) return REG_KEY_NOT_OPENED;

	while (1) {
		kNameSize = 256;
		HRESULT lRes = RegEnumKeyExA(
			hKey,
			index,
			kName,
			&kNameSize,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_NO_MORE_ITEMS) {
			if (lRes == ERROR_ACCESS_DENIED) hRes = REG_ACCESS_DENIED;
			else hRes = REG_UNKNOWN_ERROR;
		}
		if (lRes == ERROR_NO_MORE_ITEMS) break;
		kName[kNameSize] = '\0';
		callback(this, kName);
		index++;
	}
	return hRes;
}
HRESULT REGKEY::EnumAllValue(REG_VALUE_CALLBACK callback) const {
	DWORD index = 0, kNameSize = 0;
	CHAR kName[256] = "";
	HRESULT hRes = REG_SUCCESS;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	EnumValue(callback);

	while (1) {
		kNameSize = 256;
		HRESULT lRes = RegEnumKeyExA(
			hKey,
			index,
			kName,
			&kNameSize,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_NO_MORE_ITEMS) {
			if (lRes == ERROR_ACCESS_DENIED) hRes = REG_ACCESS_DENIED;
			else hRes = REG_UNKNOWN_ERROR;
		}
		if (lRes == ERROR_NO_MORE_ITEMS) break;
		kName[kNameSize] = '\0';
		REGKEY rSon;
		GetSon(kName, &rSon, ulSam);
		rSon.EnumAllValue(callback);
		rSon.Close();
		index++;
	}
	return hRes;
}
HRESULT REGKEY::EnumAllKey(REG_KEY_CALLBACK callback) const {
	DWORD index = 0, kNameSize = 0;
	CHAR kName[256] = "";
	HRESULT hRes = REG_SUCCESS;
	if (!Opened()) return REG_KEY_NOT_OPENED;

	while (1) {
		kNameSize = 256;
		HRESULT lRes = RegEnumKeyExA(
			hKey,
			index,
			kName,
			&kNameSize,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
		if (lRes != ERROR_SUCCESS && lRes != ERROR_NO_MORE_ITEMS) {
			if (lRes == ERROR_ACCESS_DENIED) hRes = REG_ACCESS_DENIED;
			else hRes = REG_UNKNOWN_ERROR;
		}
		if (lRes == ERROR_NO_MORE_ITEMS) break;
		kName[kNameSize] = '\0';
		callback(this, kName);
		REGKEY rSon;
		GetSon(kName, &rSon, ulSam);
		rSon.EnumAllKey(callback);
		rSon.Close();
		index++;
	}
	return hRes;
}

HRESULT REGKEY::SetSecurityInfo(LPCSTR lpSddl) const {
	PSECURITY_DESCRIPTOR pSD = NULL;
	if (!Opened()) return REG_KEY_NOT_OPENED;
	if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(
		lpSddl, SDDL_REVISION_1, &pSD, NULL
	)) return REG_UNKNOWN_ERROR;
	HRESULT hRes = RegSetKeySecurity(
		hKey, 
		DACL_SECURITY_INFORMATION, 
		pSD
	);
	LocalFree(pSD);
	if (hRes != ERROR_SUCCESS) {
		if (hRes == ERROR_ACCESS_DENIED) return REG_ACCESS_DENIED;
		return REG_UNKNOWN_ERROR;
	}
	return REG_SUCCESS;
}

