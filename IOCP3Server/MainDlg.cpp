#include "StdAfx.h"
#include "IOCPServer.h"
#include "MainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HWND g_hWnd = NULL;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg(): CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CMainDlg 对话框
CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CMainDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_STOP, &CMainDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDCANCEL, &CMainDlg::OnBnClickedCancel)
	ON_MESSAGE(WM_ADD_LIST_ITEM, OnAddListItem)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// CMainDlg 消息处理程序
BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// 初始化界面信息
	this->Init();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文
		SendMessage(WM_ICONERASEBKGND, 
			reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//////////////////////////////////////////////////////////////////////
// 初始化Socket库以及界面信息
void CMainDlg::Init()
{
	// 初始化Socket库
	if (!m_IOCP.LoadSocketLib())
	{
		AfxMessageBox(_T("加载Winsock 2.2失败，服务器端无法运行！"));
		PostQuitMessage(0);
	}

	// 设置本机IP地址
	SetDlgItemText(IDC_STATIC_SERVERIP, m_IOCP.GetLocalIP().c_str());
	// 设置默认端口
	SetDlgItemInt(IDC_EDIT_PORT, DEFAULT_PORT);
	// 初始化列表
	this->InitListCtrl();
	g_hWnd = this->m_hWnd;
	LPVOID pfn = (LPVOID)AddInformation;
	m_IOCP.SetLogFunc((LOG_FUNC)pfn);
}

///////////////////////////////////////////////////////////////////////
//	开始监听
void CMainDlg::OnBnClickedOk()
{
	if (!m_IOCP.Start())
	{
		AfxMessageBox(_T("服务器启动失败！"));
		return;
	}

	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_INFO);
	pList->DeleteAllItems();
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow(TRUE);
}

//////////////////////////////////////////////////////////////////////
//	结束监听
void CMainDlg::OnBnClickedStop()
{
	m_IOCP.Stop();
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
}

///////////////////////////////////////////////////////////////////////
//	初始化List Control
void CMainDlg::InitListCtrl()
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_INFO);
	pList->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	pList->InsertColumn(0, "INFORMATION", LVCFMT_LEFT, 500);
}

///////////////////////////////////////////////////////////////////////
//	点击“退出”的时候，停止监听，清空Socket类库
void CMainDlg::OnBnClickedCancel()
{
	// 停止监听
	m_IOCP.Stop();
	m_IOCP.UnloadSocketLib();
	CDialog::OnCancel();
}

///////////////////////////////////////////////////////////////////////
//	系统退出的时候，为确保资源释放，停止监听，清空Socket类库
void CMainDlg::OnDestroy()
{
	OnBnClickedCancel();
	CDialog::OnDestroy();
}

LRESULT CMainDlg::OnAddListItem(WPARAM wParam, LPARAM lParam)
{
	string* pStr = ((string*)lParam);
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_INFO);
	pList->InsertItem(0, (*pStr).c_str());
	delete pStr;
	return 0;
}

void CMainDlg::AddInformation(const string& strInfo)
{
	if (g_hWnd)
	{
		string* pStr = new string(strInfo);
		::PostMessage(g_hWnd, WM_ADD_LIST_ITEM, 0,
			(LPARAM)pStr);
	}
}