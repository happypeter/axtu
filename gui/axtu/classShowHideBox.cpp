#include "classShowHideBox.h"
#include <qtextbrowser.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcheckbox.h>

classShowHideBox::classShowHideBox() : frmShowHideBox(0,0,FALSE,Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_Title)
{	
	chkOption->setShown(false);
	btnOk->setShown(false);
	btnCancel->setShown(false);
	btnDetail->setShown(false);
	connect(btnDetail, SIGNAL(clicked()), this, SLOT(ClickedDetail()));	
	connect(btnOk, SIGNAL(clicked()), this, SLOT(ClickedOk()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(ClickedCancel()));
}

classShowHideBox::~classShowHideBox()
{
}

void classShowHideBox::ClickedDetail()
{	
	if(textList->isShown() == false)
	{	
		textList->setShown(true);
		btnDetail->setText(m_strHideButtonText);
	}
	else
	{
		textList->setShown(false);		
		btnDetail->setText(m_strShowButtonText);		
	}
}

void classShowHideBox::ClickedOk()
{
	accept();
}

void classShowHideBox::ClickedCancel()
{
	reject();
}

void classShowHideBox::SetCaption(QString strText)
{
	this->setCaption(strText);
}

void classShowHideBox::SetMessage(QString strText)
{
	labelMessage->setText(strText);
}

void classShowHideBox::SetDetailButtonText(QString strShow, QString strHide)
{	
	m_strShowButtonText = strShow;
	m_strHideButtonText = strHide;
	btnDetail->setText(strHide);
	btnDetail->setShown(true);
}

void classShowHideBox::SetOkButtonText(QString strText)
{
	btnOk->setShown(true);
	btnOk->setText(strText);
}

void classShowHideBox::SetCancelButtonText(QString strText)
{
	btnCancel->setShown(true);
	btnCancel->setText(strText);
}

void classShowHideBox::SetCheckOptionText(QString strText)
{
	chkOption->setShown(true);
	chkOption->setText(strText);
}
bool classShowHideBox::GetCheckOption()
{
	return chkOption->isChecked();
}

void classShowHideBox::ClearList(QString strText)
{
	textList->clear();
}

void classShowHideBox::AppendList(QString strText)
{
	textList->append(strText);
}

/*!
 * @brief Class header file for Graphic User Interface
 * 
 * @return 1 is Ok or Yes ,  0 is Cancel or No 
 */
int classShowHideBox::Domodal()
{
	this->exec();
}
