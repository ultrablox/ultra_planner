#ifndef ULTRADEBUGGER_H
#define ULTRADEBUGGER_H

#include <QtWidgets/QMainWindow>
#include "ui_UltraDebugger.h"
#include <solver/database/USearchDatabase.h>

class UExternalMemoryController;

class UltraDebugger : public QMainWindow
{
	Q_OBJECT

public:
	UltraDebugger(QWidget *parent = 0);
	~UltraDebugger();

	void openFile(const std::string & file_name);
	void displayDatabase(std::unique_ptr<USearchDatabase> & db);
	void displayNode(const USearchNodeReference & node_ref);
private:
	Ui::UltraDebuggerClass ui;
	std::unique_ptr<USearchDatabase> m_database;
	UExternalMemoryController * m_pMemoryCtrl;
};

#endif // ULTRADEBUGGER_H
