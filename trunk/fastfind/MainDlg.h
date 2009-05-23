// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlmisc.h>
#include <list>

class EnableControl {
public:
	CButton m_check;
	CWindow m_win;

	EnableControl(CButton check, CWindow win) :
	m_check(check), m_win(win)
	{
	}
};

class ListViewItem {
public:
	unsigned long long fileIndex;
	unsigned long long lastModifiedTime;
	unsigned long long fileSize;
	wchar_t filename[256];
	wchar_t path[MAX_PATH];

	ListViewItem() {
	}

	ListViewItem(unsigned long long fileIndex, unsigned long long lastModifiedTime, unsigned long long fileSize, wchar_t *filename, wchar_t *path) :
	fileIndex(fileIndex), lastModifiedTime(lastModifiedTime), fileSize(fileSize) {
		wcscpy(this->filename, filename);
		wcscpy(this->path, path);
	}
};

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler, public CWinDataExchange<CMainDlg>
{
private:
	CString m_strCPool[3];
	LPTSTR  m_pstrPool[3];
	int     m_nNextFree;
	LPTSTR AddPool(CString* pstr);

public:
	CEdit m_fileNameEdit;
	CDateTimePickerCtrl m_modifiedFromDate;
	CDateTimePickerCtrl m_modifiedToDate;
	CEdit m_sizeFromEdit;
    CEdit m_sizeToEdit;

	CButton m_fileNameCheck;
	CButton m_modifiedFromCheck;
	CButton m_modifiedToCheck;
	CButton m_sizeFromCheck;
	CButton m_sizeToCheck;
	CListViewCtrl m_resultsList;
	CProgressBarCtrl m_progressBar;

	EnableControl *enableControls[5];

	TCHAR m_searchPath[MAX_PATH];
	std::list<ListViewItem> lvis;

	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SIZING, OnSizing)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_FILENAME_ENABLE, BN_CLICKED, OnBnClickedEnable)
		COMMAND_HANDLER(IDC_MODIFIED_FROM_ENABLE, BN_CLICKED, OnBnClickedEnable)
		COMMAND_HANDLER(IDC_MODIFIED_TO_ENABLE, BN_CLICKED, OnBnClickedEnable)
		COMMAND_HANDLER(IDC_SIZE_FROM_ENABLE, BN_CLICKED, OnBnClickedEnable)
		COMMAND_HANDLER(IDC_SIZE_TO_ENABLE, BN_CLICKED, OnBnClickedEnable)
		COMMAND_HANDLER(IDC_BROWSE_BUTTON, BN_CLICKED, OnBnClickedBrowseButton)
		COMMAND_HANDLER(IDC_SEARCH_BUTTON, BN_CLICKED, OnBnClickedSearchButton)
		NOTIFY_HANDLER(IDC_LIST1, LVN_GETDISPINFO, OnListViewGetDispInfo)
		NOTIFY_HANDLER(IDC_LIST1, LVN_COLUMNCLICK, OnListViewColumnClick)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CMainDlg)
		DDX_CONTROL_HANDLE(IDC_FILENAME_ENABLE, m_fileNameCheck)
		DDX_CONTROL_HANDLE(IDC_MODIFIED_FROM_ENABLE, m_modifiedFromCheck)
		DDX_CONTROL_HANDLE(IDC_MODIFIED_TO_ENABLE, m_modifiedToCheck)
		DDX_CONTROL_HANDLE(IDC_SIZE_FROM_ENABLE, m_sizeFromCheck)
		DDX_CONTROL_HANDLE(IDC_SIZE_TO_ENABLE, m_sizeToCheck)

        DDX_CONTROL_HANDLE(IDC_FILENAME, m_fileNameEdit)
		DDX_CONTROL_HANDLE(IDC_MODIFIED_FROM, m_modifiedFromDate)
		DDX_CONTROL_HANDLE(IDC_MODIFIED_TO, m_modifiedToDate)
		DDX_CONTROL_HANDLE(IDC_SIZE_FROM, m_sizeFromEdit)
        DDX_CONTROL_HANDLE(IDC_SIZE_TO, m_sizeToEdit)

		DDX_TEXT(IDC_PATH_EDIT, m_searchPath)
		DDX_CONTROL_HANDLE(IDC_RESULTS_LIST, m_resultsList);
		DDX_CONTROL_HANDLE(IDC_PROGRESS1, m_progressBar);
    END_DDX_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		DoDataExchange(false);

		Init();

		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CloseDialog(0);
		return 0;
	}

	LRESULT OnBnClickedEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void Init();
	LRESULT OnBnClickedBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSearchButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSizing(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnListViewGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnListViewColumnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
};
