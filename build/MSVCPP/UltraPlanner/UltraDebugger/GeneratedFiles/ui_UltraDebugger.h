/********************************************************************************
** Form generated from reading UI file 'UltraDebugger.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ULTRADEBUGGER_H
#define UI_ULTRADEBUGGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UltraDebuggerClass
{
public:
    QAction *actionOpenDump;
    QAction *actionExit;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QWidget *wdgNodeInfo;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QListWidget *listState;
    QTableWidget *tableActions;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QTextBrowser *edStateData;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout;
    QTreeWidget *treeData;

    void setupUi(QMainWindow *UltraDebuggerClass)
    {
        if (UltraDebuggerClass->objectName().isEmpty())
            UltraDebuggerClass->setObjectName(QStringLiteral("UltraDebuggerClass"));
        UltraDebuggerClass->resize(990, 736);
        actionOpenDump = new QAction(UltraDebuggerClass);
        actionOpenDump->setObjectName(QStringLiteral("actionOpenDump"));
        actionExit = new QAction(UltraDebuggerClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        centralWidget = new QWidget(UltraDebuggerClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        wdgNodeInfo = new QWidget(centralWidget);
        wdgNodeInfo->setObjectName(QStringLiteral("wdgNodeInfo"));
        horizontalLayout = new QHBoxLayout(wdgNodeInfo);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox = new QGroupBox(wdgNodeInfo);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        listState = new QListWidget(groupBox);
        listState->setObjectName(QStringLiteral("listState"));

        gridLayout_2->addWidget(listState, 1, 1, 1, 1);

        tableActions = new QTableWidget(groupBox);
        if (tableActions->columnCount() < 3)
            tableActions->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableActions->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableActions->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableActions->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        tableActions->setObjectName(QStringLiteral("tableActions"));

        gridLayout_2->addWidget(tableActions, 1, 0, 1, 1);

        groupBox_2 = new QGroupBox(groupBox);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        edStateData = new QTextBrowser(groupBox_2);
        edStateData->setObjectName(QStringLiteral("edStateData"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(edStateData->sizePolicy().hasHeightForWidth());
        edStateData->setSizePolicy(sizePolicy);
        edStateData->setMinimumSize(QSize(0, 40));
        edStateData->setMaximumSize(QSize(16777215, 50));
        edStateData->setFrameShape(QFrame::WinPanel);
        edStateData->setFrameShadow(QFrame::Plain);
        edStateData->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edStateData->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        edStateData->setOpenLinks(false);

        verticalLayout_2->addWidget(edStateData);


        gridLayout_2->addWidget(groupBox_2, 0, 0, 1, 2);


        horizontalLayout->addWidget(groupBox);


        gridLayout->addWidget(wdgNodeInfo, 0, 0, 1, 2);

        UltraDebuggerClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(UltraDebuggerClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 990, 18));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        UltraDebuggerClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(UltraDebuggerClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        UltraDebuggerClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(UltraDebuggerClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        UltraDebuggerClass->setStatusBar(statusBar);
        dockWidget = new QDockWidget(UltraDebuggerClass);
        dockWidget->setObjectName(QStringLiteral("dockWidget"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        verticalLayout = new QVBoxLayout(dockWidgetContents);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        treeData = new QTreeWidget(dockWidgetContents);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("Address"));
        treeData->setHeaderItem(__qtreewidgetitem);
        treeData->setObjectName(QStringLiteral("treeData"));
        treeData->setUniformRowHeights(true);
        treeData->header()->setDefaultSectionSize(50);

        verticalLayout->addWidget(treeData);

        dockWidget->setWidget(dockWidgetContents);
        UltraDebuggerClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionOpenDump);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);

        retranslateUi(UltraDebuggerClass);

        QMetaObject::connectSlotsByName(UltraDebuggerClass);
    } // setupUi

    void retranslateUi(QMainWindow *UltraDebuggerClass)
    {
        UltraDebuggerClass->setWindowTitle(QApplication::translate("UltraDebuggerClass", "UltraDebugger", 0));
        actionOpenDump->setText(QApplication::translate("UltraDebuggerClass", "Open dump...", 0));
        actionOpenDump->setShortcut(QApplication::translate("UltraDebuggerClass", "Ctrl+O", 0));
        actionExit->setText(QApplication::translate("UltraDebuggerClass", "Exit", 0));
        groupBox->setTitle(QApplication::translate("UltraDebuggerClass", "Node Description", 0));
        QTableWidgetItem *___qtablewidgetitem = tableActions->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("UltraDebuggerClass", "Search Node", 0));
        QTableWidgetItem *___qtablewidgetitem1 = tableActions->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("UltraDebuggerClass", "Transition", 0));
        QTableWidgetItem *___qtablewidgetitem2 = tableActions->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("UltraDebuggerClass", "Action", 0));
        groupBox_2->setTitle(QApplication::translate("UltraDebuggerClass", "State data", 0));
        menuFile->setTitle(QApplication::translate("UltraDebuggerClass", "File", 0));
        QTreeWidgetItem *___qtreewidgetitem = treeData->headerItem();
        ___qtreewidgetitem->setText(3, QApplication::translate("UltraDebuggerClass", "Expanded", 0));
        ___qtreewidgetitem->setText(2, QApplication::translate("UltraDebuggerClass", "Parent Address", 0));
        ___qtreewidgetitem->setText(1, QApplication::translate("UltraDebuggerClass", "State ID", 0));
    } // retranslateUi

};

namespace Ui {
    class UltraDebuggerClass: public Ui_UltraDebuggerClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ULTRADEBUGGER_H
