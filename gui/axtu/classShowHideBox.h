#ifndef CLASSSHOWHIDEBOX_H_
#define CLASSSHOWHIDEBOX_H_

#include "ui/frmshowhidebox.h"

class classShowHideBox : public frmShowHideBox
{	
	Q_OBJECT
public:
	classShowHideBox();	
	virtual ~classShowHideBox();
	
	void SetCaption(QString);
	void SetMessage(QString);
	void SetDetailButtonText(QString strShow=tr("&Show"), QString strHide=tr("&Hide"));
	void SetOkButtonText(QString);
	void SetCancelButtonText(QString);
	void SetCheckOptionText(QString);
	void SetCheckOption(bool);
	bool GetCheckOption();
	void ClearList(QString);
	void AppendList(QString);
	int Domodal();
private:
	QString m_strHideButtonText;
	QString m_strShowButtonText;
public slots:
	void ClickedDetail();	
	void ClickedOk();
	void ClickedCancel();
};

#endif /*CLASSSHOWHIDEBOX_H_*/
