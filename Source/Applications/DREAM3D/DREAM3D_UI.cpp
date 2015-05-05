/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "DREAM3D_UI.h"

//-- Qt Includes
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtCore/QFileInfoList>
#include <QtCore/QDateTime>
#include <QtCore/QProcess>
#include <QtCore/QMimeData>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QListWidget>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QScrollBar>

//-- DREAM3D Includes
#include "DREAM3DLib/DREAM3DVersion.h"
#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/FilterManager.h"
#include "DREAM3DLib/FilterParameters/QFilterParametersWriter.h"

#include "QtSupportLib/ApplicationAboutBoxDialog.h"
#include "QtSupportLib/QRecentFileList.h"
#include "QtSupportLib/DREAM3DQtMacros.h"
#include "QtSupportLib/DREAM3DPluginFrame.h"
#include "QtSupportLib/HelpDialog.h"
#include "QtSupportLib/DREAM3DHelpUrlGenerator.h"

#include "DREAM3DWidgetsLib/FilterWidgetManager.h"
#include "DREAM3DWidgetsLib/UpdateCheck.h"
#include "DREAM3DWidgetsLib/UpdateCheckData.h"
#include "DREAM3DWidgetsLib/Widgets/DREAM3DUpdateCheckDialog.h"
#include "DREAM3DWidgetsLib/Widgets/PipelineViewWidget.h"
#include "DREAM3DWidgetsLib/Widgets/FilterLibraryDockWidget.h"
#include "DREAM3DWidgetsLib/Widgets/PrebuiltPipelinesDockWidget.h"
#include "DREAM3DWidgetsLib/Widgets/DREAM3DUserManualDialog.h"

#include "DREAM3D/License/DREAM3DLicenseFiles.h"

#include "AboutDREAM3D.h"
#include "AboutPlugins.h"

// Initialize private static member variable
QString DREAM3D_UI::m_OpenDialogLastDirectory = "";

namespace Detail
{
  static const QString VersionCheckGroupName("VersionCheck");
  static const QString LastVersionCheck("LastVersionCheck");
  static const QString WhenToCheck("WhenToCheck");
  static const QString UpdateWebSite("http://dream3d.bluequartz.net/version.txt");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D_UI::DREAM3D_UI(QWidget* parent) :
  QMainWindow(parent),
  m_WorkerThread(NULL),
  //  m_PluginActionGroup(NULL),
  //  m_PluginPrefsActionGroup(NULL),
  m_ActivePlugin(NULL),
  //  m_PluginToolBar(NULL),
  m_HelpDialog(NULL),
  m_UpdateCheckThread(NULL),
  m_FilterListBtn(NULL),
  m_FilterLibraryBtn(NULL),
  m_FavoritesBtn(NULL),
  m_PrebuiltBtn(NULL),
  m_IssuesBtn(NULL),
  m_ShouldRestart(false),
  m_OpenedFilePath("")
{
  m_OpenDialogLastDirectory = QDir::homePath();

  // Register all of the Filters we know about - the rest will be loaded through plugins
  //  which all should have been loaded by now.
  m_FilterManager = FilterManager::Instance();
  //m_FilterManager->RegisterKnownFilters(m_FilterManager);

  // Register all the known filterWidgets
  m_FilterWidgetManager = FilterWidgetManager::Instance();
  m_FilterWidgetManager->RegisterKnownFilterWidgets();

  // Calls the Parent Class to do all the Widget Initialization that were created
  // using the QDesigner program
  setupUi(this);

  // Do our own widget initializations
  setupGui();

  this->setAcceptDrops(true);

  // Read various settings
  readSettings();

  // Set window modified to false
  setWindowModified(false);

  // If all DREAM3D windows are closed, disable menus
  connect(qApp, SIGNAL(lastWindowClosed()), this, SLOT(disableMenuItems()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D_UI::~DREAM3D_UI()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::resizeEvent ( QResizeEvent* event )
{
  emit parentResized();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionNew_triggered()
{
  PluginManager* pluginManager = PluginManager::Instance();
  QVector<IDREAM3DPlugin*> plugins = pluginManager->getPluginsVector();

  DREAM3D_UI* newInstance = new DREAM3D_UI(NULL);
  newInstance->setLoadedPlugins(plugins);
  newInstance->setWindowTitle("[*]UntitledPipeline - DREAM3D");
  newInstance->move(this->x() + 45, this->y() + 45);
  newInstance->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionOpen_triggered()
{
  QString proposedDir = m_OpenDialogLastDirectory;
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open Pipeline"),
    proposedDir, tr("Json File (*.json);;Dream3d File (*.dream3d);;Text File (*.txt);;Ini File (*.ini);;All Files (*.*)"));
  if (true == filePath.isEmpty()) { return; }

  filePath = QDir::toNativeSeparators(filePath);
  QFileInfo fileInfo(filePath);

  PluginManager* pluginManager = PluginManager::Instance();
  QVector<IDREAM3DPlugin*> plugins = pluginManager->getPluginsVector();

  // Create new DREAM3D instance
  DREAM3D_UI* newInstance = new DREAM3D_UI(NULL);
  newInstance->setLoadedPlugins(plugins);
  newInstance->setWindowTitle("[*]" + fileInfo.baseName() + " - DREAM3D");

  // Read Pipeline
  int err = newInstance->getPipelineViewWidget()->openPipeline(filePath, Replace);

  QFileInfo fi(filePath);

  // Set Current File Path
  if (err >= 0)
  {
    newInstance->setOpenedFilePath(filePath);

    // Cache the last directory on new instance
    newInstance->setOpenDialogLastDirectory(fi.path());

    // Add file path to the recent files list for both instances
    QRecentFileList* list = QRecentFileList::instance();
    list->addFile(filePath);

    // Show the new instance
    newInstance->setWindowModified(false);
    newInstance->move(this->x() + 45, this->y() + 45);
    newInstance->show();
  }
  else
  {
    // Show error message on old DREAM3D instance
    QString errorMessage = newInstance->getPipelineViewWidget()->getStatusBar()->currentMessage();
    pipelineViewWidget->getStatusBar()->showMessage(errorMessage);

    // Delete new DREAM3D instance
    delete newInstance;
  }

  // Cache the last directory on old instance
  m_OpenDialogLastDirectory = fi.path();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionSave_triggered()
{
#if defined (Q_OS_MAC)
  QSettings::Format format = QSettings::NativeFormat;
#else
  QSettings::Format format = QSettings::IniFormat;
#endif

  if (isWindowModified() == true)
  {
    QString filePath;
    if (m_OpenedFilePath.isEmpty())
    {
      // When the file hasn't been saved before, the same functionality as a "Save As" occurs...
      on_actionSaveAs_triggered();
      return;
    }
    else
    {
      filePath = m_OpenedFilePath;
    }

    // Fix the separators
    filePath = QDir::toNativeSeparators(filePath);
    
    // Write the pipeline
    pipelineViewWidget->writePipeline(filePath);

    // Set window title and save flag
    QFileInfo prefFileInfo = QFileInfo(filePath);
    setWindowTitle("[*]" + prefFileInfo.baseName() + " - DREAM3D");
    setWindowModified(false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionSaveAs_triggered()
{
  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "Untitled.json";
  QString filePath = QFileDialog::getSaveFileName(this, tr("Save Pipeline To File"),
    proposedFile,
    tr("Json File (*.json);;DREAM3D File (*.dream3d);;All Files (*.*)"));
  if (true == filePath.isEmpty()) { return; }

  filePath = QDir::toNativeSeparators(filePath);

  //If the filePath already exists - delete it so that we get a clean write to the file
  QFileInfo fi(filePath);
  if (fi.suffix().isEmpty())
  {
    filePath.append(".json");
    fi.setFile(filePath);
  }

  // Write the pipeline
  int err = pipelineViewWidget->writePipeline(filePath);

  if (err >= 0)
  {
    // Set window title and save flag
    QFileInfo prefFileInfo = QFileInfo(filePath);
    setWindowTitle("[*]" + prefFileInfo.baseName() + " - DREAM3D");
    setWindowModified(false);

    m_OpenedFilePath = filePath;
  }

  // Cache the last directory
  m_OpenDialogLastDirectory = fi.path();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionExit_triggered()
{
#if defined (Q_OS_WIN)
  this->close();
#else
  qApp->closeAllWindows();
  qApp->quit();
#endif
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void DREAM3D_UI::closeEvent(QCloseEvent* event)
{
  QMessageBox::StandardButton choice = checkDirtyDocument();
  if (choice == QMessageBox::Cancel)
  {
    event->ignore();
    return;
  }

  disconnectSignalsSlots();

  writeSettings();
  on_actionClearPipeline_triggered();
  event->accept();

  if (m_ShouldRestart == true)
  {
    // Restart DREAM3D
    QProcess::startDetached(QApplication::applicationFilePath());
    qApp->quit();
  }
}


// -----------------------------------------------------------------------------
//  Read our settings from a file
// -----------------------------------------------------------------------------
void DREAM3D_UI::readSettings()
{
#if defined (Q_OS_MAC)
  QSettings::Format format = QSettings::NativeFormat;
#else
  QSettings::Format format = QSettings::IniFormat;
#endif
  QString filePath;
  {
    QSettings prefs(format, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());

    filePath = prefs.fileName();
    // Have the pipeline builder read its settings from the prefs file
    readWindowSettings(prefs);
    readVersionSettings(prefs);
    QRecentFileList::instance()->readList(prefs);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::readWindowSettings(QSettings& prefs)
{
  QString filePath = prefs.fileName();
  bool ok = false;
  prefs.beginGroup("WindowSettings");
  if (prefs.contains(QString("MainWindowGeometry")))
  {
    QByteArray geo_data = prefs.value(QString("MainWindowGeometry")).toByteArray();
    ok = restoreGeometry(geo_data);
    if (!ok)
    {
      qDebug() << "Error Restoring the Window Geometry" << "\n";
    }
  }

  if (prefs.contains(QString("MainWindowState")))
  {
    std::cout << "Reading State of Main Window" << std::endl;
    QByteArray layout_data = prefs.value(QString("MainWindowState")).toByteArray();
    restoreState(layout_data);
  }

  readDockWidgetSettings(prefs, filterListDockWidget);
  readDockWidgetSettings(prefs, filterLibraryDockWidget);
  favoritesDockWidget->readSettings(this, prefs);
  readDockWidgetSettings(prefs, prebuiltPipelinesDockWidget);
  readDockWidgetSettings(prefs, issuesDockWidget);

  readSearchListSettings(prefs, filterListDockWidget);

  QByteArray splitterGeometry = prefs.value(QString("Splitter_Geometry")).toByteArray();
  splitter->restoreGeometry(splitterGeometry);
  QByteArray splitterSizes = prefs.value(QString("Splitter_Sizes")).toByteArray();
  splitter->restoreState(splitterSizes);

  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::readDockWidgetSettings(QSettings& prefs, QDockWidget* dw)
{
  restoreDockWidget(dw);

  QString name = dw->objectName();
  bool b = prefs.value(dw->objectName(), false).toBool();
  dw->setHidden(b);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::readSearchListSettings(QSettings& prefs, FilterListDockWidget* dw)
{
  QString objectName = prefs.value("ActiveSearchAction").toString();
  QList<QAction*> list = dw->getSearchActionList();

  bool didCheck = false;
  for (int i = 0; i < list.size(); i++)
  {
    if (list[i]->objectName() == objectName)
    {
      list[i]->setChecked(true);
      didCheck = true;
    }
    else
    {
      list[i]->setChecked(false);
    }
  }

  if (didCheck == false && list.size() > 0)
  {
    // Set "All Words" as checked by default
    list[0]->setChecked(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::readVersionSettings(QSettings& prefs)
{
  
}

// -----------------------------------------------------------------------------
//  Write our Prefs to file
// -----------------------------------------------------------------------------
void DREAM3D_UI::writeSettings()
{
#if defined (Q_OS_MAC)
  QSettings::Format format = QSettings::NativeFormat;
#else
  QSettings::Format format = QSettings::IniFormat;
#endif
  // We scope these sections so that the QSettings object goes out of scope each
  // time and is destructed
  {
    QSettings prefs(format, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());

    // Have the pipeline builder write its settings to the prefs file
    writeWindowSettings(prefs);
    // Have the version check widet write its preferences.
    writeVersionCheckSettings(prefs);

    QRecentFileList::instance()->writeList(prefs);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::writeVersionCheckSettings(QSettings& prefs)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::writeWindowSettings(QSettings& prefs)
{
  prefs.beginGroup("WindowSettings");
  QByteArray geo_data = saveGeometry();
  QByteArray layout_data = saveState();
  prefs.setValue(QString("MainWindowGeometry"), geo_data);
  prefs.setValue(QString("MainWindowState"), layout_data);

  writeDockWidgetSettings(prefs, filterListDockWidget);
  writeDockWidgetSettings(prefs, filterLibraryDockWidget);
  favoritesDockWidget->writeSettings(prefs);
  writeDockWidgetSettings(prefs, prebuiltPipelinesDockWidget);
  writeDockWidgetSettings(prefs, issuesDockWidget);

  writeSearchListSettings(prefs, filterListDockWidget);

  QByteArray splitterGeometry = splitter->saveGeometry();
  QByteArray splitterSizes = splitter->saveState();
  prefs.setValue(QString("Splitter_Geometry"), splitterGeometry);
  prefs.setValue(QString("Splitter_Sizes"), splitterSizes);

  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::writeDockWidgetSettings(QSettings& prefs, QDockWidget* dw)
{
  prefs.setValue(dw->objectName(), dw->isHidden() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::writeSearchListSettings(QSettings& prefs, FilterListDockWidget* dw)
{
  prefs.setValue("ActiveSearchAction", dw->getActiveSearchAction()->objectName());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::checkForUpdatesAtStartup()
{
  DREAM3DUpdateCheckDialog* d = new DREAM3DUpdateCheckDialog(this);
  if ( d->getAutomaticallyBtn()->isChecked() )
  {
#if defined (Q_OS_MAC)
    QSettings updatePrefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
    QSettings updatePrefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif

    updatePrefs.beginGroup( DREAM3DUpdateCheckDialog::getUpdatePreferencesGroup() );
    QDate lastUpdateCheckDate = updatePrefs.value(DREAM3DUpdateCheckDialog::getUpdateCheckKey()).toDate();
    updatePrefs.endGroup();

    QDate systemDate;
    QDate currentDateToday = systemDate.currentDate();

    QDate dailyThreshold = lastUpdateCheckDate.addDays(1);
    QDate weeklyThreshold = lastUpdateCheckDate.addDays(7);
    QDate monthlyThreshold = lastUpdateCheckDate.addMonths(1);

    if ( (d->getHowOftenComboBox()->currentIndex() == DREAM3DUpdateCheckDialog::UpdateCheckDaily && currentDateToday >= dailyThreshold)
         || (d->getHowOftenComboBox()->currentIndex() == DREAM3DUpdateCheckDialog::UpdateCheckWeekly && currentDateToday >= weeklyThreshold)
         || (d->getHowOftenComboBox()->currentIndex() == DREAM3DUpdateCheckDialog::UpdateCheckMonthly && currentDateToday >= monthlyThreshold) )
    {
      m_UpdateCheck = QSharedPointer<UpdateCheck>(new UpdateCheck(this));

      connect(m_UpdateCheck.data(), SIGNAL( LatestVersion(UpdateCheckData*) ),
              this, SLOT( versionCheckReply(UpdateCheckData*) ) );

      m_UpdateCheck->checkVersion(Detail::UpdateWebSite);
    }
  }


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::setupGui()
{
  // Automatically check for updates at startup if the user has indicated that preference before
  checkForUpdatesAtStartup();

  m_HelpDialog = new HelpDialog(this);
  m_HelpDialog->setWindowModality(Qt::NonModal);

  pipelineViewWidget->setScrollArea(pipelineViewScrollArea);

  // Stretch Factors
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);

  pipelineViewScrollArea->verticalScrollBar()->setSingleStep(5);

  // Make the connections between the gui elements
  QRecentFileList* recentsList = QRecentFileList::instance();

  // Hook up the signals from the various docks to the PipelineViewWidget that will either add a filter
  // or load an entire pipeline into the view
  connectSignalsSlots();

  pipelineViewWidget->setStatusBar(statusbar);

  // This will set the initial list of filters in the filterListDockWidget
  // Tell the Filter Library that we have more Filters (potentially)
  filterLibraryDockWidget->refreshFilterGroups();

  // Set the IssuesDockWidget as a PipelineMessageObserver Object.
  pipelineViewWidget->setPipelineMessageObserver(issuesDockWidget);

  setupViewMenu();
  setupPipelineContextMenu();

  if(favoritesDockWidget)
  {
    favoritesDockWidget->configureFilterLibraryTree();
  }

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::disconnectSignalsSlots()
{
  QRecentFileList* recentsList = QRecentFileList::instance();
  disconnect(filterLibraryDockWidget, SIGNAL(filterItemDoubleClicked(const QString&)),
          pipelineViewWidget, SLOT(addFilter(const QString&)) );

  disconnect(filterListDockWidget, SIGNAL(filterItemDoubleClicked(const QString&)),
          pipelineViewWidget, SLOT(addFilter(const QString&)) );

  disconnect(prebuiltPipelinesDockWidget, SIGNAL(pipelineFileActivated(QString, ExtractionType)),
          pipelineViewWidget, SLOT(openPipeline(QString, ExtractionType)));

  disconnect(prebuiltPipelinesDockWidget, SIGNAL(pipelineFileActivated(QString, ExtractionType)),
    this, SLOT(pipelineFileLoaded(QString, ExtractionType)));

  disconnect(favoritesDockWidget, SIGNAL(pipelineFileActivated(const QString&, ExtractionType)),
    pipelineViewWidget, SLOT(openPipeline(const QString&, ExtractionType)));

  disconnect(favoritesDockWidget, SIGNAL(pipelineFileActivated(QString, ExtractionType)),
    this, SLOT(pipelineFileLoaded(QString, ExtractionType)));

  disconnect(favoritesDockWidget, SIGNAL(pipelineNeedsToBeSaved(const QString&, const QString&)),
    pipelineViewWidget, SLOT(updateFavorite(const QString&, const QString&)));

  disconnect(recentsList, SIGNAL(fileListChanged(const QString &)),
    this, SLOT(updateRecentFileList(const QString &)));

  disconnect(pipelineViewWidget, SIGNAL(filterInputWidgetChanged(FilterInputWidget*)),
    this, SLOT(setFilterInputWidget(FilterInputWidget*)));

  disconnect(pipelineViewWidget, SIGNAL(noFilterWidgetsInPipeline()),
    this, SLOT(clearFilterInputWidget()));

  disconnect(pipelineViewWidget, SIGNAL(filterParameterChanged()),
    this, SLOT(markDocumentAsDirty()));
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::connectSignalsSlots()
{
  QRecentFileList* recentsList = QRecentFileList::instance();
  connect(filterLibraryDockWidget, SIGNAL(filterItemDoubleClicked(const QString&)),
          pipelineViewWidget, SLOT(addFilter(const QString&)) );

  connect(filterListDockWidget, SIGNAL(filterItemDoubleClicked(const QString&)),
          pipelineViewWidget, SLOT(addFilter(const QString&)) );

  connect(prebuiltPipelinesDockWidget, SIGNAL(pipelineFileActivated(QString, ExtractionType)),
    pipelineViewWidget, SLOT(openPipeline(QString, ExtractionType)));

  connect(prebuiltPipelinesDockWidget, SIGNAL(pipelineFileActivated(QString, ExtractionType)),
    this, SLOT(pipelineFileLoaded(QString, ExtractionType)));

  connect(favoritesDockWidget, SIGNAL(pipelineFileActivated(const QString&, ExtractionType)),
    pipelineViewWidget, SLOT(openPipeline(const QString&, ExtractionType)));

  connect(favoritesDockWidget, SIGNAL(pipelineFileActivated(QString, ExtractionType)),
    this, SLOT(pipelineFileLoaded(QString, ExtractionType)));

  connect(favoritesDockWidget, SIGNAL(pipelineNeedsToBeSaved(const QString&, const QString&)),
    pipelineViewWidget, SLOT(updateFavorite(const QString&, const QString&)));

  connect(recentsList, SIGNAL(fileListChanged(const QString &)),
    this, SLOT(updateRecentFileList(const QString &)));

  connect(pipelineViewWidget, SIGNAL(filterInputWidgetChanged(FilterInputWidget*)),
    this, SLOT(setFilterInputWidget(FilterInputWidget*)));

  connect(pipelineViewWidget, SIGNAL(noFilterWidgetsInPipeline()),
    this, SLOT(clearFilterInputWidget()));

  connect(pipelineViewWidget, SIGNAL(filterParameterChanged()),
    this, SLOT(markDocumentAsDirty()));
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::setupPipelineContextMenu()
{
  QList<QAction*> favoriteItemActions;
  QList<QAction*> favoriteCategoryActions;
  QList<QAction*> favoriteDefaultActions;
  QList<QAction*> prebuiltItemActions;
  QList<QAction*> prebuildCategoryActions;


  /* ******************************* Favorites Pipelines Context Menus ***********************************************/

  QAction* actionAddFavorite = new QAction(menuPipeline);
  actionAddFavorite->setObjectName(QString::fromUtf8("actionAddFavorite"));
  actionAddFavorite->setText(QApplication::translate("DREAM3D_UI", "Add Favorite", 0));
  menuPipeline->addAction(actionAddFavorite);
  QKeySequence actionAddFavKeySeq(Qt::CTRL + Qt::Key_F);
  actionAddFavorite->setShortcut(actionAddFavKeySeq);
  connect(actionAddFavorite, SIGNAL(triggered()),
          favoritesDockWidget, SLOT( actionAddFavorite_triggered() ) );
  favoriteItemActions << actionAddFavorite;
  favoriteCategoryActions << actionAddFavorite;
  favoriteDefaultActions << actionAddFavorite;

  QAction* actionAddFavoriteFolder = new QAction(menuPipeline);
  actionAddFavoriteFolder->setObjectName(QString::fromUtf8("actionAddFavoriteFolder"));
  actionAddFavoriteFolder->setText(QApplication::translate("DREAM3D_UI", "Add Favorite Folder", 0));
  menuPipeline->addAction(actionAddFavoriteFolder);
  QKeySequence actionAddFolderKeySeq(Qt::CTRL + Qt::SHIFT + Qt::Key_F);
  actionAddFavoriteFolder->setShortcut(actionAddFolderKeySeq);
  connect(actionAddFavoriteFolder, SIGNAL(triggered()),
          favoritesDockWidget, SLOT( actionAddFavoriteFolder_triggered() ) );
  favoriteItemActions << actionAddFavoriteFolder;
  favoriteCategoryActions << actionAddFavoriteFolder;
  favoriteDefaultActions << actionAddFavoriteFolder;


  QAction* actionUpdateFavorite = new QAction(menuPipeline);
  actionUpdateFavorite->setObjectName(QString::fromUtf8("actionUpdateFavorite"));
  actionUpdateFavorite->setText(QApplication::translate("DREAM3D_UI", "Update Favorite", 0));
  menuPipeline->addAction(actionUpdateFavorite);
  QKeySequence actionUpdateFavKeySeq(Qt::CTRL + Qt::Key_U);
  actionUpdateFavorite->setShortcut(actionUpdateFavKeySeq);
  connect(actionUpdateFavorite, SIGNAL(triggered()),
          favoritesDockWidget, SLOT( actionUpdateFavorite_triggered() ) );
  favoriteItemActions << actionUpdateFavorite;

  QAction* actionRenameFavorite = new QAction(menuPipeline);
  actionRenameFavorite->setObjectName(QString::fromUtf8("actionRenameFavorite"));
  actionRenameFavorite->setText(QApplication::translate("DREAM3D_UI", "Rename Favorite", 0));
  menuPipeline->addAction(actionRenameFavorite);
  QKeySequence actionRenameFavKeySeq(Qt::CTRL + Qt::Key_R);
  actionRenameFavorite->setShortcut(actionRenameFavKeySeq);
  connect(actionRenameFavorite, SIGNAL(triggered()),
          favoritesDockWidget, SLOT( actionRenameFavorite_triggered() ) );
  favoriteItemActions << actionRenameFavorite;
  favoriteCategoryActions << actionRenameFavorite;


  QAction* actionAppendFavorite = new QAction(NULL);
  actionAppendFavorite->setObjectName(QString::fromUtf8("actionAppendFavorite"));
  actionAppendFavorite->setText(QApplication::translate("DREAM3D_UI", "Append Favorite to Pipeline", 0));
   menuPipeline->addAction(actionAppendFavorite);
  QKeySequence actionAppendFavKeySeq(Qt::CTRL + Qt::Key_A);
  actionAppendFavorite->setShortcut(actionAppendFavKeySeq);
  connect(actionAppendFavorite, SIGNAL(triggered()),
          favoritesDockWidget, SLOT( actionAppendFavorite_triggered() ) );
  favoriteItemActions << actionAppendFavorite;
  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    favoriteItemActions << separator;
    favoriteCategoryActions << separator;
    menuPipeline->addSeparator();
  }

  QAction* actionRemoveFavorite = new QAction(menuPipeline);
  actionRemoveFavorite->setObjectName(QString::fromUtf8("actionRemoveFavorite"));
  actionRemoveFavorite->setText(QApplication::translate("DREAM3D_UI", "Delete Favorite", 0));
  menuPipeline->addAction(actionRemoveFavorite);
  QKeySequence actionRemoveFavKeySeq(Qt::CTRL + Qt::Key_Delete);
  actionRemoveFavorite->setShortcut(actionRemoveFavKeySeq);
  connect(actionRemoveFavorite, SIGNAL(triggered()),
          favoritesDockWidget, SLOT( actionRemoveFavorite_triggered() ) );
  favoritesDockWidget->setDeleteAction(actionRemoveFavorite);
  favoriteItemActions << actionRemoveFavorite;
  favoriteCategoryActions << actionRemoveFavorite;

  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    favoriteItemActions << separator;
    favoriteCategoryActions << separator;
    menuPipeline->addSeparator();
  }


  {
    QAction* actionShowInFileSystem = new QAction(this);
    actionShowInFileSystem->setObjectName(QString::fromUtf8("actionShowInFileSystem"));
    // Handle the naming based on what OS we are currently running...
#if defined(Q_OS_WIN)
    actionShowInFileSystem->setText(QApplication::translate("DREAM3D_UI", "Show in Windows Explorer", 0));
#elif defined(Q_OS_MAC)
    actionShowInFileSystem->setText(QApplication::translate("DREAM3D_UI", "Show in Finder", 0));
#else
    actionShowInFileSystem->setText(QApplication::translate("DREAM3D_UI", "Show in File System", 0));
#endif

    connect(actionShowInFileSystem, SIGNAL(triggered()),
            favoritesDockWidget, SLOT( actionShowInFileSystem_triggered() ) );
    {
      QAction* separator = new QAction(this);
      separator->setSeparator(true);
      favoriteItemActions << separator;
      prebuiltItemActions << separator;
    }
    favoriteItemActions << actionShowInFileSystem;
    favoriteCategoryActions << actionShowInFileSystem;
  }
  favoritesDockWidget->getFilterLibraryTreeWidget()->setNodeActionList(favoriteCategoryActions);
  favoritesDockWidget->getFilterLibraryTreeWidget()->setLeafActionList(favoriteItemActions);
  favoritesDockWidget->getFilterLibraryTreeWidget()->setDefaultActionList(favoriteDefaultActions);



  /* ******************************* Prebuilt Pipelines Context Menus ***********************************************/
  QAction* actionAppendPrebuilt = new QAction(NULL);
  actionAppendPrebuilt->setObjectName(QString::fromUtf8("actionAppendPrebuilt"));
  actionAppendPrebuilt->setText(QApplication::translate("DREAM3D_UI", "Append Prebuilt to Pipeline", 0));
  //menuPipeline->addAction(actionAppendPrebuilt);
  connect(actionAppendPrebuilt, SIGNAL(triggered()),
          prebuiltPipelinesDockWidget, SLOT( actionAppendPipeline_triggered() ) );
  prebuiltItemActions << actionAppendPrebuilt;

  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    prebuiltItemActions << separator;
    menuPipeline->addSeparator();
  }

  {
    QAction* actionShowInFileSystem = new QAction(this);
    actionShowInFileSystem->setObjectName(QString::fromUtf8("actionShowInFileSystem"));
    // Handle the naming based on what OS we are currently running...
#if defined(Q_OS_WIN)
    actionShowInFileSystem->setText(QApplication::translate("DREAM3D_UI", "Show in Windows Explorer", 0));
#elif defined(Q_OS_MAC)
    actionShowInFileSystem->setText(QApplication::translate("DREAM3D_UI", "Show in Finder", 0));
#else
    actionShowInFileSystem->setText(QApplication::translate("DREAM3D_UI", "Show in File System", 0));
#endif

    connect(actionShowInFileSystem, SIGNAL(triggered()),
            prebuiltPipelinesDockWidget, SLOT( actionShowInFileSystem_triggered() ) );
    {
      QAction* separator = new QAction(this);
      separator->setSeparator(true);
      favoriteItemActions << separator;
      prebuiltItemActions << separator;
    }

    prebuiltItemActions << actionShowInFileSystem;
    prebuildCategoryActions << actionShowInFileSystem;
  }

  prebuiltPipelinesDockWidget->getFilterLibraryTreeWidget()->setNodeActionList(prebuildCategoryActions);
  prebuiltPipelinesDockWidget->getFilterLibraryTreeWidget()->setLeafActionList(prebuiltItemActions);

  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    menuPipeline->addSeparator();
  }
  QAction* actionClearPipeline = new QAction(menuPipeline);
  actionClearPipeline->setObjectName(QString::fromUtf8("actionClearPipeline"));
  actionClearPipeline->setText(QApplication::translate("DREAM3D_UI", "Clear Pipeline", 0));
  menuPipeline->addAction(actionClearPipeline);
  QKeySequence actionClearKeySeq(Qt::CTRL + Qt::Key_Escape);
  actionClearPipeline->setShortcut(actionClearKeySeq);
  connect(actionClearPipeline, SIGNAL(triggered()),
          this, SLOT( on_actionClearPipeline_triggered() ) );


  /* ******************************* PipelineView Actions Setup ***********************************************/
  QList<QAction*> pipelineViewActions;
  pipelineViewActions << actionAddFavorite << actionAddFavoriteFolder;
  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    pipelineViewActions << separator;
  }
  pipelineViewActions <<  actionClearPipeline;
  pipelineViewWidget->setContextMenuActions(pipelineViewActions);

  /* ******************************* PipelineView Actions Setup ***********************************************/


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::setupViewMenu()
{
  //  m_FilterListBtn = new QToolButton(this);
  //  makeStatusBarButton("Filters", filterListDockWidget, m_FilterListBtn, 0);
  menuView->removeAction(actionShow_Filter_List);
  delete actionShow_Filter_List;
  actionShow_Filter_List = filterListDockWidget->toggleViewAction();
  actionShow_Filter_List->setText("Filter List");

  menuView->addAction(actionShow_Filter_List);
  connect(actionShow_Filter_List, SIGNAL(triggered(bool)),
          this, SLOT(on_actionShow_Filter_List_triggered(bool)) );
  //  connect(m_FilterListBtn, SIGNAL(toggled(bool)),
  //          this, SLOT(on_actionShow_Filter_List_triggered(bool)) );


  //  m_FilterLibraryBtn = new QToolButton(this);
  //  makeStatusBarButton("Filter Library", filterLibraryDockWidget, m_FilterLibraryBtn, 1);
  menuView->removeAction(actionShow_Filter_Library);
  delete actionShow_Filter_Library;
  actionShow_Filter_Library = filterLibraryDockWidget->toggleViewAction();
  actionShow_Filter_Library->setText("Filter Library");
  menuView->addAction(actionShow_Filter_Library);
  connect(actionShow_Filter_Library, SIGNAL(triggered(bool)),
          this, SLOT(on_actionShow_Filter_Library_triggered(bool)) );
  //  connect(m_FilterLibraryBtn, SIGNAL(toggled(bool)),
  //          this, SLOT(on_actionShow_Filter_Library_triggered(bool)) );

  //  m_FavoritesBtn = new QToolButton(this);
  //  makeStatusBarButton("Favorites", favoritesDockWidget, m_FavoritesBtn, 2);
  menuView->removeAction(actionShow_Favorites);
  delete actionShow_Favorites;
  actionShow_Favorites = favoritesDockWidget->toggleViewAction();
  actionShow_Favorites->setText("Favorite Pipelines");
  menuView->addAction(actionShow_Favorites);
  connect(actionShow_Favorites, SIGNAL(triggered(bool)),
          this, SLOT(on_actionShow_Favorites_triggered(bool)) );
  //  connect(m_FavoritesBtn, SIGNAL(toggled(bool)),
  //          this, SLOT(on_actionShow_Favorites_triggered(bool)) );

  //  m_PrebuiltBtn = new QToolButton(this);
  //  makeStatusBarButton("Prebuilt", prebuiltPipelinesDockWidget, m_PrebuiltBtn, 3);
  menuView->removeAction(actionShow_Prebuilt_Pipelines);
  delete actionShow_Prebuilt_Pipelines;
  actionShow_Prebuilt_Pipelines = prebuiltPipelinesDockWidget->toggleViewAction();
  actionShow_Prebuilt_Pipelines->setText("Prebuilt Pipelines");
  menuView->addAction(actionShow_Prebuilt_Pipelines);
  connect(actionShow_Prebuilt_Pipelines, SIGNAL(triggered(bool)),
          this, SLOT(on_actionShow_Prebuilt_Pipelines_triggered(bool)) );
  //  connect(m_PrebuiltBtn, SIGNAL(toggled(bool)),
  //          this, SLOT(on_actionShow_Prebuilt_Pipelines_triggered(bool)) );


  //  m_IssuesBtn = new QToolButton(this);
  //  makeStatusBarButton("Issues", issuesDockWidget, m_IssuesBtn, 4);
  menuView->removeAction(actionShow_Issues);
  delete actionShow_Issues;
  actionShow_Issues = issuesDockWidget->toggleViewAction();
  actionShow_Issues->setText("Show Warnings/Errors");
  menuView->addAction(actionShow_Issues);
  connect(actionShow_Issues, SIGNAL(triggered(bool)),
          this, SLOT(on_actionShow_Issues_triggered(bool)) );
  //  connect(m_IssuesBtn, SIGNAL(toggled(bool)),
  //          this, SLOT(on_actionShow_Issues_triggered(bool)) );

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::makeStatusBarButton(QString text, QDockWidget* dockWidget, QToolButton* btn, int index)
{
  btn->setText(text);
  btn->setCheckable(true);
  btn->setChecked(!dockWidget->isHidden());
  statusBar()->insertPermanentWidget(index, btn, 0);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::setLoadedPlugins(QVector<IDREAM3DPlugin*> plugins)
{
  m_LoadedPlugins = plugins;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::pipelineFileLoaded(QString file, ExtractionType type)
{
  QFileInfo fi(file);
  on_pipelineViewWidget_pipelineTitleUpdated(fi.baseName());
  setWindowFilePath(file);
  setWindowModified(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_pipelineViewWidget_pipelineFileDropped(QString& file)
{
  pipelineFileLoaded(file, Replace);
  m_OpenedFilePath = file;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_pipelineViewWidget_pipelineChanged()
{
  setWindowModified(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_pipelineViewWidget_pipelineTitleUpdated(QString title)
{
  setWindowTitle(QString("[*]") + title + " - DREAM3D");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_pipelineViewWidget_pipelineIssuesCleared()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_pipelineViewWidget_pipelineHasNoErrors()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionPlugin_Information_triggered()
{
  AboutPlugins dialog(this);
  dialog.exec();

  // Write cache on exit
  dialog.writePluginCache();

  /* If any of the load checkboxes were changed, display a dialog warning
   * the user that they must restart DREAM3D to see the changes.
   */
  if (dialog.getLoadPreferencesDidChange() == true)
  {
    QMessageBox msgBox;
    msgBox.setText("DREAM3D must be restarted to allow these changes to take effect.");
    msgBox.setInformativeText("Restart?");
    msgBox.setWindowTitle("Restart Needed");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int choice = msgBox.exec();

    if (choice == QMessageBox::Yes)
    {
      // Set Restart Flag and Begin closing DREAM3D
      m_ShouldRestart = true;
      this->close();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionCheck_For_Updates_triggered()
{
  DREAM3DUpdateCheckDialog* d = new DREAM3DUpdateCheckDialog(this);

  d->setCurrentVersion((DREAM3DLib::Version::Complete()));
  d->setUpdateWebSite(Detail::UpdateWebSite);
  d->setApplicationName("DREAM3D");

  // Read from the QSettings Pref file the information that we need
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif
  prefs.beginGroup(Detail::VersionCheckGroupName);
  QDateTime dateTime = prefs.value(Detail::LastVersionCheck, QDateTime::currentDateTime()).toDateTime();
  d->setLastCheckDateTime(dateTime);
  prefs.endGroup();

  // Now display the dialog box
  d->exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::dragEnterEvent(QDragEnterEvent* e)
{
  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  QDir parent(file);
  this->m_OpenDialogLastDirectory = parent.dirName();
  QFileInfo fi(file );
  QString ext = fi.suffix();
  if (fi.exists() && fi.isFile() && ( ext.compare("mxa") || ext.compare("h5") || ext.compare("hdf5") ) )
  {
    e->accept();
  }
  else
  {
    e->ignore();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::dropEvent(QDropEvent* e)
{
  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  QDir parent(file);
  this->m_OpenDialogLastDirectory = parent.dirName();
  QFileInfo fi(file );
  QString ext = fi.suffix();
  file = QDir::toNativeSeparators(file);
  if (fi.exists() && fi.isFile() )
  {
    //TODO: INSERT Drop Event CODE HERE
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMessageBox::StandardButton DREAM3D_UI::checkDirtyDocument()
{
  qint32 err = -1;

  if (this->isWindowModified() == true)
  {
    int r = QMessageBox::warning(this, tr("DREAM.3D"),
                                 tr("The Pipeline has been modified.\nDo you want to save your changes?"),
                                 QMessageBox::Save | QMessageBox::Default,
                                 QMessageBox::Discard,
                                 QMessageBox::Cancel | QMessageBox::Escape);
    if (r == QMessageBox::Save)
    {
      on_actionSave_triggered();
      return QMessageBox::Save;
    }
    else if (r == QMessageBox::Discard)
    {
      return QMessageBox::Discard;
    }
    else if (r == QMessageBox::Cancel)
    {
      return QMessageBox::Cancel;
    }
  }
  
  return QMessageBox::Ignore;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::updateRecentFileList(const QString &file)
{
  // Clear the Recent Items Menu
  this->menu_RecentFiles->clear();

  // Get the list from the static object
  QStringList files = QRecentFileList::instance()->fileList();
  foreach (QString file, files)
  {
    QAction* action = new QAction(this->menu_RecentFiles);
    action->setText(QRecentFileList::instance()->parentAndFileName(file));
    action->setData(file);
    action->setVisible(true);
    this->menu_RecentFiles->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
  }

  this->menu_RecentFiles->addSeparator();
  this->menu_RecentFiles->addAction(actionClearRecentFiles);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::openRecentFile()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (action)
  {
    //qDebug() << "Opening Recent file: " << action->data().toString() << "\n";
    QString filePath = action->data().toString();
    int err = getPipelineViewWidget()->openPipeline(filePath);

    if (err >= 0)
    {
      QFileInfo fileInfo(filePath);
      setWindowTitle("[*]" + fileInfo.baseName() + " - DREAM3D");
      setOpenedFilePath(filePath);
      setWindowModified(false);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_startPipelineBtn_clicked()
{

  if (startPipelineBtn->text().compare("Cancel") == 0)
  {
    qDebug() << "canceling from GUI...." << "\n";
    emit pipelineCanceled();
    //    m_WorkerThread->wait(); // Wait until the thread is complete
    //    if (m_WorkerThread->isFinished() == true)
    //    {
    //      delete m_WorkerThread;
    //      m_WorkerThread = NULL;
    //    }
    return;
  }

  if (m_WorkerThread != NULL)
  {
    m_WorkerThread->wait(); // Wait until the thread is complete
    if (m_WorkerThread->isFinished() == true)
    {
      delete m_WorkerThread;
      m_WorkerThread = NULL;
    }
  }
  m_WorkerThread = new QThread(); // Create a new Thread Resource


  // Clear out the Issues Table
  issuesDockWidget->clearIssues();

  // Ask the PipelineViewWidget to create a FilterPipeline Object
  m_PipelineInFlight = pipelineViewWidget->getCopyOfFilterPipeline();

  // Give the pipeline one last chance to preflight and get all the latest values from the GUI
  int err = m_PipelineInFlight->preflightPipeline();
  if(err < 0)
  {
    m_PipelineInFlight = FilterPipeline::NullPointer();
    return;
  }

  // Save the preferences file NOW in case something happens
  writeSettings();


  pipelineViewWidget->setEnabled(false);

  // Move the FilterPipeline object into the thread that we just created.
  m_PipelineInFlight->moveToThread(m_WorkerThread);

  // Allow the GUI to receive messages - We are only interested in the progress messages
  m_PipelineInFlight->addMessageReceiver(this);

  /* Connect the signal 'started()' from the QThread to the 'run' slot of the
   * PipelineBuilder object. Since the PipelineBuilder object has been moved to another
   * thread of execution and the actual QThread lives in *this* thread then the
   * type of connection will be a Queued connection.
   */
  // When the thread starts its event loop, start the PipelineBuilder going
  connect(m_WorkerThread, SIGNAL(started()),
          m_PipelineInFlight.get(), SLOT(run()));

  // When the PipelineBuilder ends then tell the QThread to stop its event loop
  connect(m_PipelineInFlight.get(), SIGNAL(pipelineFinished() ),
          m_WorkerThread, SLOT(quit()) );

  // When the QThread finishes, tell this object that it has finished.
  connect(m_WorkerThread, SIGNAL(finished()),
          this, SLOT( pipelineDidFinish() ) );

  // If the use clicks on the "Cancel" button send a message to the PipelineBuilder object
  // We need a Direct Connection so the
  connect(this, SIGNAL(pipelineCanceled() ),
          m_PipelineInFlight.get(), SLOT (cancelPipeline() ), Qt::DirectConnection);



  emit pipelineStarted();
  m_WorkerThread->start();
  startPipelineBtn->setText("Cancel");

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::populateMenus(QObject* plugin)
{

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::processPipelineMessage(const PipelineMessage& msg)
{
  switch(msg.getType())
  {
    case PipelineMessage::ProgressValue:
      this->m_ProgressBar->setValue(msg.getProgressValue());
      break;
    case PipelineMessage::StatusMessage:
      if(NULL != this->statusBar())
      {
        QString s = (msg.getPrefix());
        s = s.append(" ").append(msg.getText().toLatin1().data());
        this->statusBar()->showMessage(s);
      }
      break;
    case PipelineMessage::StatusMessageAndProgressValue:
      this->m_ProgressBar->setValue(msg.getProgressValue());
      if(NULL != this->statusBar())
      {
        QString s = (msg.getPrefix());
        s = s.append(" ").append(msg.getText().toLatin1().data());
        this->statusBar()->showMessage(s);
      }
      break;

    default:
      return;
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::pipelineDidFinish()
{
  std::cout << "DREAM3D_UI::pipelineDidFinish()" << std::endl;
  m_PipelineInFlight = FilterPipeline::NullPointer();// This _should_ remove all the filters and deallocate them
  startPipelineBtn->setText("Go");
  m_ProgressBar->setValue(0);
  pipelineViewWidget->setEnabled(true);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::displayHelp(QString file)
{
  m_HelpDialog->setContentFile(file);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionAbout_DREAM3D_triggered()
{
  AboutDREAM3D d(this);
  d.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionLicense_Information_triggered()
{
  ApplicationAboutBoxDialog about(DREAM3D::LicenseList, this);
  QString an = QCoreApplication::applicationName();
  QString version("");
  version.append(DREAM3DLib::Version::PackageComplete().toLatin1().data());
  about.setApplicationInfo(an, version);
  about.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionShowIndex_triggered()
{
  // Generate help page
  QUrl helpURL = DREAM3DHelpUrlGenerator::generateHTMLUrl("index");
  DREAM3DUserManualDialog::LaunchHelpDialog(helpURL);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::versionCheckReply(UpdateCheckData* dataObj)
{
  DREAM3DUpdateCheckDialog* d = new DREAM3DUpdateCheckDialog(this);
  d->setCurrentVersion((DREAM3DLib::Version::Complete()));
  d->setApplicationName("DREAM3D");

  if ( dataObj->hasUpdate() && !dataObj->hasError() )
  {
    QString message = dataObj->getMessageDescription();
    QLabel* feedbackTextLabel = d->getFeedbackTextLabel();
    d->toSimpleUpdateCheckDialog();
    feedbackTextLabel->setText(message);
    d->getCurrentVersionLabel()->setText( dataObj->getAppString() );
    d->setCurrentVersion( dataObj->getAppString() );
    d->getLatestVersionLabel()->setText( dataObj->getServerString() );
    d->exec();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionClearPipeline_triggered()
{
  // Clear the filter input widget
  clearFilterInputWidget();

  pipelineViewWidget->clearWidgets();
  setWindowTitle("[*]Untitled Pipeline");
  setWindowModified(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionClearRecentFiles_triggered()
{
  // Clear the Recent Items Menu
  this->menu_RecentFiles->clear();

  this->menu_RecentFiles->addSeparator();
  this->menu_RecentFiles->addAction(actionClearRecentFiles);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionShow_Filter_Library_triggered(bool b)
{
  updateAndSyncDockWidget(actionShow_Filter_Library, filterLibraryDockWidget, m_FilterLibraryBtn, b);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionShow_Filter_List_triggered(bool b)
{
  updateAndSyncDockWidget(actionShow_Filter_List, filterListDockWidget, m_FilterListBtn, b);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionShow_Prebuilt_Pipelines_triggered(bool b)
{
  updateAndSyncDockWidget(actionShow_Prebuilt_Pipelines, prebuiltPipelinesDockWidget, m_PrebuiltBtn, b);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionShow_Favorites_triggered(bool b)
{
  updateAndSyncDockWidget(actionShow_Favorites, favoritesDockWidget, m_FavoritesBtn, b);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::on_actionShow_Issues_triggered(bool b)
{
  updateAndSyncDockWidget(actionShow_Issues, issuesDockWidget, m_IssuesBtn, b);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::updateAndSyncDockWidget(QAction* action, QDockWidget* dock, QToolButton* btn, bool b)
{
  if(m_FilterListBtn == NULL || m_FilterLibraryBtn == NULL || m_FavoritesBtn == NULL || m_PrebuiltBtn == NULL || m_IssuesBtn == NULL) { return; }

  action->blockSignals(true);
  dock->blockSignals(true);
  btn->blockSignals(true);

  action->setChecked(b);
  btn->toggle();
  dock->setVisible(b);
#if 0
  if(b == false)
  {
    QString text = action->text().replace("Show", "Hide");
    action->setText(text);
  }
  else
  {
    QString text = action->text().replace("Hide", "Show");
    action->setText(text);
  }
#endif
  action->blockSignals(false);
  dock->blockSignals(false);
  btn->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewWidget* DREAM3D_UI::getPipelineViewWidget()
{
  return pipelineViewWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::setOpenedFilePath(const QString &filePath)
{
  m_OpenedFilePath = filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::setOpenDialogLastDirectory(const QString &path)
{
  m_OpenDialogLastDirectory = path;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::setFilterInputWidget(FilterInputWidget* widget)
{
  // Clear the filter input widget
  clearFilterInputWidget();

  // Set the widget into the frame
  fiwFrameVLayout->addWidget(widget);
  widget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::clearFilterInputWidget()
{
  QLayoutItem* item = fiwFrameVLayout->takeAt(0);
  if (item)
  {
    QWidget* w = item->widget();
    if (w)
    {
      w->hide();
      w->setParent(NULL);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::disableMenuItems()
{
  menuPipeline->setDisabled(true);
  menuView->setDisabled(true);
  actionSave->setDisabled(true);
  actionSaveAs->setDisabled(true);
  actionPlugin_Information->setDisabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DREAM3D_UI::markDocumentAsDirty()
{
  setWindowModified(true);
}



