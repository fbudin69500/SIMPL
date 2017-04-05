/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "SVPipelineFilterWidget.h"

#include <QtCore/QDir>
#include <QtCore/QMimeData>
#include <QtCore/QResource>
#include <QtCore/QTimer>

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGroupBox>

#include "SVWidgetsLib/QtSupport/QtSStyles.h"

#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/Common/FilterFactory.hpp"
#include "SIMPLib/Common/FilterManager.h"
#include "SIMPLib/Common/IFilterFactory.hpp"
#include "SIMPLib/Common/IObserver.h"
#include "SIMPLib/FilterParameters/DataContainerReaderFilterParameter.h"
#include "SIMPLib/FilterParameters/InputFileFilterParameter.h"
#include "SIMPLib/FilterParameters/InputPathFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedChoicesFilterParameter.h"

#include "SVWidgetsLib/Core/FilterWidgetManager.h"
#include "SVWidgetsLib/FilterParameterWidgets/ChoiceWidget.h"
#include "SVWidgetsLib/FilterParameterWidgets/LinkedBooleanWidget.h"
#include "SVWidgetsLib/Widgets/DataContainerArrayWidget.h"
#include "SVWidgetsLib/Widgets/SVPipelineViewWidget.h"

#define PADDING 5
#define BORDER 2
#define IMAGE_WIDTH 17
#define IMAGE_HEIGHT 17

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_SVPipelineFilterWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineFilterWidget::SVPipelineFilterWidget(QWidget* parent)
: QFrame(parent)
, PipelineFilterObject(AbstractFilter::NullPointer())
, m_Observer(nullptr)
, m_Selected(false)
, m_HasRightClickTarget(false)
{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineFilterWidget::SVPipelineFilterWidget(AbstractFilter::Pointer filter, IObserver* observer, QWidget* parent)
: QFrame(parent)
, PipelineFilterObject(filter)
, m_Observer(observer)
, m_Selected(false)
, m_HasRightClickTarget(false)
{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineFilterWidget::~SVPipelineFilterWidget()
{
  if(m_Observer != nullptr)
  {
    delete m_Observer;
    m_Observer = nullptr;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::initialize()
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(getFilterInputWidget(), SIGNAL(filterParametersChanged()), this, SLOT(filterInputWidget_filterParametersChanged()));
  connect(getFilterInputWidget(), SIGNAL(errorSettingFilterParameter(const QString&)), this, SLOT(displayFilterParameterWidgetError(const QString&)));

  setupUi(this);

  m_DeleteRect.setX(PADDING + BORDER);
  m_DeleteRect.setY(PADDING + BORDER);
  m_DeleteRect.setWidth(IMAGE_WIDTH);
  m_DeleteRect.setHeight(IMAGE_HEIGHT);

  // Set the Name of the filter into the FilterWidget
  filterName->setText(getFilter()->getHumanLabel());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SVPipelineFilterWidget::isSelected()
{
  return m_Selected;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::setSelected(bool s)
{
  m_Selected = s;
  changeStyle();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SVPipelineFilterWidget::hasRightClickTarget()
{
  return m_HasRightClickTarget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::setHasRightClickTarget(bool value)
{
  m_HasRightClickTarget = value;
  changeStyle();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::adjustLayout(QWidget* w, int state)
{
  if(state == Qt::Checked)
  {
    w->show();
  }
  else
  {
    w->hide();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::displayFilterParameterWidgetError(const QString& msg)
{
  if(m_Observer)
  {
    PipelineMessage pm("Filter Paramter Widget", msg, -1, PipelineMessage::Error);
    m_Observer->processPipelineMessage(pm);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::setFilterTitle(const QString title)
{
  filterName->setText(title);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::setFilterIndex(int i, int numFilters)
{
  if (numFilters < 10) { numFilters = 11; }
  QString numStr = QString::number(i);

  if(numFilters > 9)
  {
    int mag = 0;
    int max = numFilters;
    while(max > 0)
    {
      mag++;
      max = max / 10;
    }
    numStr = "";             // Clear the string
    QTextStream ss(&numStr); // Create a QTextStream to set up the padding
    ss.setFieldWidth(mag);
    ss.setPadChar('0');
    ss << i;
  }

  filterIndex->setText(numStr);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::changeStyle()
{

  QString svWidgetStyle;
  QTextStream svWidgetStyleStream(&svWidgetStyle);

  // Define Operating Specific Font Size here
#if defined(Q_OS_WIN)
  QString fontString =  QString("font: 100 12pt \"") + QtSStyles::GetUIFont() + QString("\";");
#elif defined(Q_OS_MAC)
  QString fontString =  QString("font: 100 14pt \"") + QtSStyles::GetUIFont() + QString("\";");
#else
  QString fontString =  QString("font: 100 9pt \"") + QtSStyles::GetUIFont() + QString("\";");
#endif

  // Style the over all widget
  svWidgetStyleStream << "SVPipelineFilterWidget {\n";
  svWidgetStyleStream << "border-radius: 3px;";
  svWidgetStyleStream << "padding: 0 0 0 0px;";
  svWidgetStyleStream << "font-weight: bold;";
  svWidgetStyleStream << "background-color: rgb(185, 185, 185);";
  svWidgetStyleStream << fontString;

  if(isSelected() == true && isFocused() == true)
  {
    svWidgetStyleStream << "border: 2px solid rgb(2, 0, 216);";
  }
  else if(isSelected() == true && isFocused() == false)
  {
    svWidgetStyleStream << "border: 2px solid rgb(51, 141, 203);";
  }
  else
  {
    svWidgetStyleStream << "margin: 2px;";
  }

  if(isSelected() == false && hasRightClickTarget() == true)
  {
    svWidgetStyleStream << "border-style: dotted;";
  }

  svWidgetStyleStream << "}\n";

  svWidgetStyleStream << "QLabel {";
  svWidgetStyleStream << fontString;
  svWidgetStyleStream << "}";

  // Set the Style Sheet
  setStyleSheet(svWidgetStyle);

  // Style the Left side of the widget where the Filter Index is displayed.
  QString filterIndexStyle;
  QTextStream filterIndexStyleStream(&filterIndexStyle);

  filterIndexStyleStream << "QLabel\n{";
  filterIndexStyleStream << "color: rgb(242, 242, 242);";
  filterIndexStyleStream << "padding: 3 3 3 3px;";
  //filterIndexStyleStream << "border-radius: 3px;";

  filterIndexStyleStream << fontString;



  if(getHasPreflightWarnings())
  {
    filterIndexStyleStream << "background-color: rgb(172, 168, 0);";
  }
  else if( getHasPreflightErrors())
  {
    filterIndexStyleStream << "background-color: rgb(179, 2, 5);";
  }
  else
  {
//    filterIndexStyleStream << "background-color: rgb(51, 141, 203);";
#if 1
    filterIndexStyleStream << "background-color: rgb(48, 48, 48);";
#else
    float increment = 300.0f / 14.0f;
    int saturation = 160;
    int value = 178;
    int i = 0;
    QString humanName = getFilter()->getGroupName();
    qDebug() << humanName;
    if(SIMPL::FilterGroups::CoreFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(25, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::GenericFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(50, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::OrientationAnalysisFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(75, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::IOFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(100, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::ProcessingFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(125, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::ReconstructionFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(150, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::SamplingFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(175, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::StatisticsFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(200, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::SyntheticBuildingFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(225, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::SurfaceMeshingFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(250, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::SolidMeshingFilters.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(275, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::Utilities.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(300 * increment, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else if(SIMPL::FilterGroups::Unsupported.compare(humanName) == 0)
    {
      QColor c = QColor::fromHsv(325 * increment, saturation, value, 255);
      filterIndexStyleStream << "background-color: rgb(" << c.red() << "," << c.green() << "," << c.blue() << ");";
    }
    else
    {
      filterIndexStyleStream << "background-color: rgb(68, 68, 68);";
    }
#endif

  }

  // Set the style sheet for the Left hand side of the widget
  filterIndexStyleStream << "}\n";
  filterIndex->setStyleSheet(filterIndexStyle);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::RightButton)
  {
    QFrame::mousePressEvent(event);
  }
  else if(event->button() == Qt::LeftButton)
  {
    dragStartPosition = event->pos();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::mouseReleaseEvent(QMouseEvent* event)
{
  emit filterWidgetPressed(this, qApp->queryKeyboardModifiers());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::mouseMoveEvent(QMouseEvent* event)
{
  if(!(event->buttons() & Qt::LeftButton))
  {
    return;
  }
  if((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
  {
    return;
  }

  emit dragStarted(event, this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::focusInEvent(QFocusEvent* event)
{
  emit focusInEventStarted(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::focusOutEvent(QFocusEvent* event)
{
  emit focusOutEventStarted(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::getGuiParametersFromFilter(AbstractFilter* filt)
{
  Q_ASSERT("SVPipelineFilterWidget::getGuiParametersFromFilter executed"); // Code should never enter this function
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::on_deleteBtn_clicked()
{
  emit filterWidgetRemoved(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::filterInputWidget_filterParametersChanged()
{
  emit parametersChanged(QUuid());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::launchHelpForItem()
{
  QString className = getFilterClassName();

  DocRequestManager* docRequester = DocRequestManager::Instance();
  docRequester->requestFilterDocs(className);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::toRunningState()
{
  getFilterInputWidget()->toRunningState(); // getVariablesTabContentsWidget()->setDisabled(true);
  deleteBtn->setDisabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineFilterWidget::toIdleState()
{
  getFilterInputWidget()->toIdleState(); // getVariablesTabContentsWidget()->setEnabled(true);
  deleteBtn->setEnabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineFilterObject* SVPipelineFilterWidget::deepCopy()
{
  SVPipelineFilterWidget* newWidget = new SVPipelineFilterWidget(getFilter()->newFilterInstance(true), nullptr, dynamic_cast<QWidget*>(parent()));
  newWidget->setHasPreflightErrors(getHasPreflightErrors());
  newWidget->setHasPreflightWarnings(getHasPreflightWarnings());

  return newWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QHBoxLayout* SVPipelineFilterWidget::getHorizontalLayout()
{
  return horizontalLayout;
}
