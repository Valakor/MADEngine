#ifndef ANGERMANAGEMENT_H
#define ANGERMANAGEMENT_H

#include <QtWidgets/QMainWindow>
#include "ui_angermanagement.h"

class AngerManagement : public QMainWindow
{
	Q_OBJECT

public:
	AngerManagement(QWidget *parent = 0);
	~AngerManagement();

private:
	Ui::AngerManagementClass ui;
};

#endif // ANGERMANAGEMENT_H
