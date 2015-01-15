
#include "UltraDebugger.h"
#include <QFileDialog>
#include <QtWidgets/QMessageBox>
#include <solver/UExternalMemoryController.h>

class SearchNodeTreeItem : public QTreeWidgetItem
{
public:
	SearchNodeTreeItem(const QStringList & lst, const USearchNodeReference & ref)
		:QTreeWidgetItem(lst), nodeRef(ref)
	{
	}

	USearchNodeReference nodeRef;
};

UltraDebugger::UltraDebugger(QWidget *parent)
	: QMainWindow(parent)
{
	m_pMemoryCtrl = new UExternalMemoryController("C", "tmp");
	ui.setupUi(this);

	connect(ui.actionExit, &QAction::triggered, this, &QMainWindow::close);

	connect(ui.actionOpenDump, &QAction::triggered, [=](){
		auto dump_bile_name = QFileDialog::getOpenFileName(this, tr("Open UltraSolver Dump"), "", tr("UltraSolver dump (*.usd)"));
		if(!dump_bile_name.isEmpty())
			openFile(dump_bile_name.toStdString());
	});

	connect(ui.treeData, &QTreeWidget::itemActivated, [=](QTreeWidgetItem * item, int column){
		auto nit = dynamic_cast<SearchNodeTreeItem*>(item);
		if(nit)
			displayNode(nit->nodeRef);
	});
}

UltraDebugger::~UltraDebugger()
{
}

void UltraDebugger::openFile(const std::string & file_name)
{
	unique_ptr<USearchDatabase> db(new USearchDatabase());
	auto err = db->loadFromDump(file_name);
	if(err)
	{
		QMessageBox::critical(this, tr("Ultra Debugger"), tr("Unable to load database dump."), QMessageBox::Ok);
		return;
	}

	//Display loaded DB
	displayDatabase(db);
}

void UltraDebugger::displayDatabase(std::unique_ptr<USearchDatabase> & db)
{
	swap(m_database, db);
	

	for(auto & node_gr : m_database->data())
	{
		QStringList item_data;
		item_data << QString::fromStdString(to_string(node_gr.first)) << "" << "" << "";

		auto gr_item = new QTreeWidgetItem(item_data);


		for(auto node_it = m_database->groupBegin(node_gr.first), end_it = m_database->groupEnd(node_gr.first); node_it != end_it; ++node_it)
		{
			QStringList node_item_data;
			node_item_data << QString::fromStdString(to_string(node_it.address())) << QString("%1").arg(node_it.stateIndex()) << (node_it.hasParent() ? QString::fromStdString(to_string(node_it.parentAddress())) : "-") << (node_it.expanded() ? "yes" : "no");

			auto node_item = new SearchNodeTreeItem(node_item_data, node_it);
			gr_item->addChild(node_item);
		}
		/*for(auto node_data : node_gr.second)
		{
			

			//node_item_data << to_string(node_data.parentIndex) << 
			
		}*/


		ui.treeData->addTopLevelItem(gr_item);
	}
}

void UltraDebugger::displayNode(const USearchNodeReference & node_ref)
{
	//Display node
	auto state = (*m_database)[node_ref.stateIndex()];
	ui.edStateData->setPlainText(QString::fromStdString(to_string(state)));


	//Transition path
	auto path = m_database->buildTransitionPath(node_ref);

	ui.tableActions->setRowCount(path.size());
	for(int i = 0; i < path.size(); ++i)
	{
		//Node
		ui.tableActions->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(to_string(path[i].second.address()))));

		//Transition
		int transition_index = path[i].first;
		ui.tableActions->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(transition_index)));

		//Action
		ui.tableActions->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m_database->transitionSystem().transition(transition_index).description())));
	}

	//State
	ui.listState->clear();
	auto state_descr = m_database->transitionSystem().interpretState(state);
	for(auto & sd : state_descr)
	{
		ui.listState->addItem(new QListWidgetItem(QString::fromStdString(sd)));
	}
}
