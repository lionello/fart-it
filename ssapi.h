// Machine generated IDispatch wrapper class(es) created with ClassWizard
/////////////////////////////////////////////////////////////////////////////
// IVSSItem wrapper class

class IVSSItem : public COleDispatchDriver
{
public:
	IVSSItem() {}		// Calls COleDispatchDriver default constructor
	IVSSItem(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSSItem(const IVSSItem& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	CString GetSpec();
	BOOL GetBinary();
	void SetBinary(BOOL bNewValue);
	BOOL GetDeleted();
	void SetDeleted(BOOL bNewValue);
	long GetType();
	CString GetLocalSpec();
	void SetLocalSpec(LPCTSTR lpszNewValue);
	CString GetName();
	void SetName(LPCTSTR lpszNewValue);
	LPDISPATCH GetParent();
	long GetVersionNumber();
	LPDISPATCH GetItems(BOOL IncludeDeleted);
	void Get(BSTR* Local, long iFlags);
	void Checkout(LPCTSTR Comment, LPCTSTR Local, long iFlags);
	void Checkin(LPCTSTR Comment, LPCTSTR Local, long iFlags);
	void UndoCheckout(LPCTSTR Local, long iFlags);
	long GetIsCheckedOut();
	LPDISPATCH GetCheckouts();
	BOOL GetIsDifferent(LPCTSTR Local);
	LPDISPATCH Add(LPCTSTR Local, LPCTSTR Comment, long iFlags);
	LPDISPATCH NewSubproject(LPCTSTR Name, LPCTSTR Comment);
	void Share(LPDISPATCH pIItem, LPCTSTR Comment, long iFlags);
	void Destroy();
	void Move(LPDISPATCH pINewParent);
	void Label(LPCTSTR Label, LPCTSTR Comment);
	LPDISPATCH GetVersions(long iFlags);
	LPDISPATCH GetVersion(const VARIANT& Version);
};
/////////////////////////////////////////////////////////////////////////////
// IVSSVersions wrapper class

class IVSSVersions : public COleDispatchDriver
{
public:
	IVSSVersions() {}		// Calls COleDispatchDriver default constructor
	IVSSVersions(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSSVersions(const IVSSVersions& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	LPUNKNOWN _NewEnum();
};
/////////////////////////////////////////////////////////////////////////////
// IVSSVersion wrapper class

class IVSSVersion : public COleDispatchDriver
{
public:
	IVSSVersion() {}		// Calls COleDispatchDriver default constructor
	IVSSVersion(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSSVersion(const IVSSVersion& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	CString GetUsername();
	long GetVersionNumber();
	CString GetAction();
	DATE GetDate();
	CString GetComment();
	CString GetLabel();
	LPDISPATCH GetVSSItem();
};
/////////////////////////////////////////////////////////////////////////////
// IVSSItems wrapper class

class IVSSItems : public COleDispatchDriver
{
public:
	IVSSItems() {}		// Calls COleDispatchDriver default constructor
	IVSSItems(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSSItems(const IVSSItems& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	long GetCount();
	LPDISPATCH GetItem(const VARIANT& sItem);
	LPUNKNOWN _NewEnum();
};
/////////////////////////////////////////////////////////////////////////////
// IVSSCheckouts wrapper class

class IVSSCheckouts : public COleDispatchDriver
{
public:
	IVSSCheckouts() {}		// Calls COleDispatchDriver default constructor
	IVSSCheckouts(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSSCheckouts(const IVSSCheckouts& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	long GetCount();
	LPDISPATCH GetItem(const VARIANT& sItem);
	LPUNKNOWN _NewEnum();
};
/////////////////////////////////////////////////////////////////////////////
// IVSSCheckout wrapper class

class IVSSCheckout : public COleDispatchDriver
{
public:
	IVSSCheckout() {}		// Calls COleDispatchDriver default constructor
	IVSSCheckout(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSSCheckout(const IVSSCheckout& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	CString GetUsername();
	DATE GetDate();
	CString GetLocalSpec();
	CString GetMachine();
	CString GetProject();
	CString GetComment();
	long GetVersionNumber();
};
/////////////////////////////////////////////////////////////////////////////
// IVSSDatabase wrapper class

class IVSSDatabase : public COleDispatchDriver
{
public:
	IVSSDatabase() {}		// Calls COleDispatchDriver default constructor
	IVSSDatabase(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSSDatabase(const IVSSDatabase& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	void Open(LPCTSTR SrcSafeIni, LPCTSTR Username, LPCTSTR Password);
	CString GetSrcSafeIni();
	CString GetDatabaseName();
	CString GetUsername();
	CString GetCurrentProject();
	void SetCurrentProject(LPCTSTR lpszNewValue);
	LPDISPATCH GetVSSItem(LPCTSTR Spec, BOOL Deleted);
};
/////////////////////////////////////////////////////////////////////////////
// IVSS wrapper class

class IVSS : public COleDispatchDriver
{
public:
	IVSS() {}		// Calls COleDispatchDriver default constructor
	IVSS(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IVSS(const IVSS& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	// method 'QueryInterface' not emitted because of invalid return type or parameter type
	unsigned long AddRef();
	unsigned long Release();
	// method 'GetTypeInfoCount' not emitted because of invalid return type or parameter type
	// method 'GetTypeInfo' not emitted because of invalid return type or parameter type
	// method 'GetIDsOfNames' not emitted because of invalid return type or parameter type
	// method 'Invoke' not emitted because of invalid return type or parameter type
	LPDISPATCH GetVSSDatabase();
};
