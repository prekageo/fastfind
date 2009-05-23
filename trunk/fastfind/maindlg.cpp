#include "StdAfx.h"
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlmisc.h>
#include "resource.h"
#include "MainDlg.h"

#include "../ntfs/ntfs.h"

#define ELEMENTS(x) (sizeof((x))/sizeof(*(x)))

typedef struct {
	int nColumn;
	BOOL bAscending;
} SORTINFO;

void CMainDlg::Init() {
	this->enableControls[0] = new EnableControl(this->m_fileNameCheck, this->m_fileNameEdit);
	this->enableControls[1] = new EnableControl(this->m_modifiedFromCheck, this->m_modifiedFromDate);
	this->enableControls[2] = new EnableControl(this->m_modifiedToCheck, this->m_modifiedToDate);
	this->enableControls[3] = new EnableControl(this->m_sizeFromCheck, this->m_sizeFromEdit);
	this->enableControls[4] = new EnableControl(this->m_sizeToCheck, this->m_sizeToEdit);

	m_searchPath[0] = 0;

	m_resultsList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	m_resultsList.AddColumn(_T("Name"), 0);
	m_resultsList.AddColumn(_T("In Folder"), 1);
	m_resultsList.AddColumn(_T("Size"), 2);
	m_resultsList.AddColumn(_T("Type"), 3);
	m_resultsList.AddColumn(_T("Date Modified"), 4);

	m_nNextFree = 0;

	this->DoDataExchange();
}

LRESULT CMainDlg::OnBnClickedEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for (int i=0;i<ELEMENTS(this->enableControls);i++) {
		EnableControl *e = this->enableControls[i];
		e->m_win.EnableWindow(e->m_check.GetCheck());
	}

	return 0;
}

LRESULT CMainDlg::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint p(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	this->ClientToScreen(&p);

	for (int i=0;i<ELEMENTS(this->enableControls);i++) {
		EnableControl *e = this->enableControls[i];
		CRect rect;
		e->m_win.GetWindowRect(&rect);
		if (rect.PtInRect(p)) {
			e->m_check.SetCheck(TRUE);
			e->m_win.EnableWindow(TRUE);
			e->m_win.SetFocus();
			break;
		}
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFolderDialog folderDlg;
	DoDataExchange(TRUE);
	folderDlg.SetInitialFolder(this->m_searchPath);
	if (folderDlg.DoModal() == IDOK) {
		_tcscpy_s(this->m_searchPath, ELEMENTS(this->m_searchPath), folderDlg.GetFolderPath());
		this->DoDataExchange();
	}

	return 0;
}

unsigned long long DateTimePickerToFileTime(CDateTimePickerCtrl dateTimePicker) {
	SYSTEMTIME sysTime;
	FILETIME fileTime;
	ULARGE_INTEGER ulint;

	dateTimePicker.GetSystemTime(&sysTime);
	SystemTimeToFileTime(&sysTime, &fileTime);
	ulint.LowPart = fileTime.dwLowDateTime;
	ulint.HighPart = fileTime.dwHighDateTime;
	return ulint.QuadPart;
}

int CompareInt64(unsigned long long x, unsigned long long y) {
	if (x>y)
		return 1;
	else if (x<y)
		return -1;
	return 0;
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	ListViewItem *pItem1 = (ListViewItem*) lParam1;
	ListViewItem *pItem2 = (ListViewItem*) lParam2;
	SORTINFO* pSortInfo = (SORTINFO*) lParamSort;

	int result;
	switch (pSortInfo->nColumn) {
		case 0:
			result = wcscmp(pItem1->filename, pItem2->filename);
			break;
		case 1:
			result = wcscmp(pItem1->path, pItem2->path);
			break;
		case 2:
			result = CompareInt64(pItem1->fileSize, pItem2->fileSize);
			break;
		case 3:
			wchar_t *dot1, *dot2;
			dot1 = wcsrchr(pItem1->filename, '.');
			dot2 = wcsrchr(pItem2->filename, '.');
			if (dot1 == NULL) dot1 = L"(no)";
			if (dot2 == NULL) dot2 = L"(no)";
			result = wcscmp(dot1, dot2);
			break;
		case 4:
			result = CompareInt64(pItem1->lastModifiedTime, pItem2->lastModifiedTime);
			break;
		default:
			break;
	}

	if (!pSortInfo->bAscending) {
		result = -result;
	}

	return result;
}

DWORD WINAPI SearchThread(LPVOID lpParameter) {
	CMainDlg* dlg = (CMainDlg*)lpParameter;

	dlg->DoDataExchange(TRUE);

	if (dlg->m_searchPath[0] == 0 || dlg->m_searchPath[1] != ':') {
		MessageBox(NULL, _T("Invalid search path."), _T("Fast Find"), MB_OK | MB_ICONERROR);
		return 0;
	}
	DWORD x = GetFileAttributes(dlg->m_searchPath);
	if (x == INVALID_FILE_ATTRIBUTES || (x & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		MessageBox(NULL, _T("Invalid search path."), _T("Fast Find"), MB_OK | MB_ICONERROR);
		return 0;
	};

	TCHAR buffer[256];
    struct FIND_PARAMS findParams;
	char device[7] = "\\\\.\\x:";
	findParams.device = device;
	findParams.device[4] = (char)dlg->m_searchPath[0];
	wcscpy(findParams.rootDir, &dlg->m_searchPath[2]);
	findParams.mask = 0;
	if (dlg->m_fileNameCheck.GetCheck()) {
		findParams.mask |= FIND_MASK_FILENAME;
		dlg->m_fileNameEdit.GetWindowTextW(findParams.filename, ELEMENTS(findParams.filename));
	}
	if (dlg->m_modifiedFromCheck.GetCheck()) {
		findParams.mask |= FIND_MASK_MODIFIEDFROM;
		findParams.modifiedFrom = DateTimePickerToFileTime(dlg->m_modifiedFromDate);
	}
	if (dlg->m_modifiedToCheck.GetCheck()) {
		findParams.mask |= FIND_MASK_MODIFIEDTO;
		findParams.modifiedTo = DateTimePickerToFileTime(dlg->m_modifiedToDate);
	}
	if (dlg->m_sizeFromCheck.GetCheck()) {
		findParams.mask |= FIND_MASK_SIZEFROM;
		dlg->m_sizeFromEdit.GetWindowTextW(buffer, ELEMENTS(buffer));
		findParams.sizeFrom = 1024LL * _ttoi(buffer);
	}
	if (dlg->m_sizeToCheck.GetCheck()) {
		findParams.mask |= FIND_MASK_SIZETO;
		dlg->m_sizeToEdit.GetWindowTextW(buffer, ELEMENTS(buffer));
		findParams.sizeTo = 1024LL * _ttoi(buffer);
	}

	long long filesCount;
	FindStart(&findParams, &filesCount);

	dlg->m_progressBar.SetRange32(0, (int)filesCount);

	struct FIND_RESULT findResult;
	int i=0;

	dlg->m_resultsList.DeleteAllItems();
	dlg->lvis.clear();
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.pszText = LPSTR_TEXTCALLBACK;
	while (int ret = FindNext(&findResult)) {
		if (ret == 1) {
			ListViewItem lvi(findResult.fileIndex, findResult.lastModifiedTime, findResult.fileSize, findResult.filename, L"");
			dlg->lvis.push_back(lvi);

			lvItem.lParam = (LPARAM)&dlg->lvis.back();
			dlg->m_resultsList.InsertItem(&lvItem);
			lvItem.iItem++;
		}
		dlg->m_progressBar.SetPos((int)findResult.fileIndex);
		CString titleText;
		titleText.Format(_T("[%d%%] Fast Find"), 100LL*findResult.fileIndex/filesCount);
		dlg->SetWindowTextW(titleText);
	}
	dlg->m_progressBar.SetPos((int)findResult.fileIndex);
	dlg->SetWindowTextW(_T("Fast Find"));

	dlg->m_resultsList.LockWindowUpdate();
	i=0;
	for (std::list<ListViewItem>::iterator lvi = dlg->lvis.begin();lvi != dlg->lvis.end();) {
		if (FindFilter(lvi->fileIndex)) {
			FindGetPath(lvi->fileIndex, lvi->path);
			dlg->m_resultsList.Update(i++);
			++lvi;
		} else {
			lvi = dlg->lvis.erase(lvi);
			dlg->m_resultsList.DeleteItem(i);
		}
	}
	dlg->m_resultsList.LockWindowUpdate(FALSE);

	NtfsFindClose();

	return 0;
}

LRESULT CMainDlg::OnBnClickedSearchButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CreateThread(NULL, 0, SearchThread, this, 0, NULL);
	return 0;
}

LRESULT CMainDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	CRect wndRect(0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	CRect rect;
	m_resultsList.GetWindowRect(rect);
	this->ScreenToClient(rect);
	rect.right = wndRect.right - rect.left;
	rect.bottom = wndRect.bottom - rect.left;
	m_resultsList.MoveWindow(rect);

	return 0;
}

LRESULT CMainDlg::OnSizing(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	CRect wndRect((RECT*)lParam);

	int minWidth = 590;
	int minHeight = 300;
	if (wndRect.Width() < minWidth) {
		wndRect.right=wndRect.left+minWidth;
	}
	if (wndRect.Height() < minHeight) {
		wndRect.bottom=wndRect.top+minHeight;
	}
	CopyRect((RECT*)lParam, wndRect);

	return 0;
}

LRESULT CMainDlg::OnListViewGetDispInfo(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pnmh;

	if (pDispInfo->item.mask & LVIF_TEXT) {
		CString strField;

		ListViewItem* lvi = (ListViewItem*) pDispInfo->item.lParam;
		switch (pDispInfo->item.iSubItem) {
			case 0:
				strField = lvi->filename;
				break;
			case 1:
				strField = lvi->path;
				break;
			case 2:
				strField.Format(_T("%I64u KB"), lvi->fileSize/1024);
				break;
			case 3:
				wchar_t *dot;
				dot = wcsrchr(lvi->filename, '.');
				if (dot == NULL) {
					dot = L"(no)";
				}
				strField = dot;
				break;
			case 4:
				TCHAR buffer[64];
				ULARGE_INTEGER ulint;
				FILETIME fileTime;
				SYSTEMTIME sysTime;
				ulint.QuadPart = lvi->lastModifiedTime;
				fileTime.dwHighDateTime = ulint.HighPart;
				fileTime.dwLowDateTime = ulint.LowPart;
				FileTimeToSystemTime(&fileTime, &sysTime);
				GetDateFormat(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, buffer, ELEMENTS(buffer));
				strField = buffer;
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, buffer, ELEMENTS(buffer));
				strField += " ";
				strField += buffer;
				break;
		}

		LPTSTR pstrBuffer = AddPool(&strField);
		pDispInfo->item.pszText = pstrBuffer;
	}

	return 0;
}

LPTSTR CMainDlg::AddPool(CString* pstr)
{
	LPTSTR pstrRetVal;
	int nOldest = m_nNextFree;

	m_strCPool[m_nNextFree] = *pstr;
	pstrRetVal = m_strCPool[m_nNextFree].LockBuffer();
	m_pstrPool[m_nNextFree++] = pstrRetVal;
	m_strCPool[nOldest].ReleaseBuffer();

	if (m_nNextFree == 3)
		m_nNextFree = 0;
	return pstrRetVal;
}

LRESULT CMainDlg::OnListViewColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLISTVIEW* nmListView = (NMLISTVIEW*)pnmh;

	SORTINFO si;
	si.nColumn = nmListView->iSubItem;
	si.bAscending = 1;

	this->m_resultsList.SortItems(CompareFunc, (LPARAM) &si);

	return 0;
}
