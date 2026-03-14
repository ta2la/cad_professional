//
// Copyright (C) 2018 Petr Talla. [petr.talla@gmail.com]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//		      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=============================================================================
#include "T2lCmd_object_select.h"
#include <T2lScene.h>
#include <T2lUpdateLock.h>
#include <T2lRef.h>
#include <T2lDisplay.h>
#include <T2lRefColSelection.h>
#include <T2lGObject.h>
#include <T2lGObjectPool.h>
#include "T2lPoint2.h"
#include "T2lPoint2.h"
#include "T2lCadObject_image.h"
#include "T2lGFile.h"
#include "T2lEntityLine.h"
#include "T2lEntityPoint.h"
#include "T2lFilterFile.h"
#include "T2lCadSettings.h"

#include <QDir>
#include <QCoreApplication>
#include <QTextStream>

using namespace T2l;

//===================================================================
Cmd_object_select::Cmd_object_select(void) :
    CmdCad("select"),
    lastSelected_(nullptr)
{
}

//===================================================================
Cmd_object_select::~Cmd_object_select(void)
{
}

//===================================================================
void Cmd_object_select::enterPoint( const Point2F& pt, Display& view )
{
    //<STEP> Searching scene.
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    UpdateLock l;

    pack->cleanDynamic();
    pack->dynamicRefresh();

    foundClean();

    if (CAD_SETTINGS.selectMode() == CadSettings::SEL_RECT) {
        if ( cmdpts_.count() >= 1)
        {
            Box2F box;
            Point2F p0 = cmdpts_.get(0);
            Point2F p1 = pt;
            box.inflateTo(p0);
            box.inflateTo(p1);

            for ( long i = 0; i < pack->scene()->count(); i++ )
            {
                Ref* ref = pack->scene()->get(i);
                ObjectDisplable* displable = dynamic_cast<ObjectDisplable*>(ref->object());
                if (displable == nullptr) continue;
                if (displable->parent_ == nullptr) continue;

                GFile* activeFile = ActiveFile::activeGet()->file();
                if ( activeFile != displable->parent_) continue;

                //if ( dynamic_cast<CadObject_image*>(displable) != nullptr ) continue;

                bool cancel = false;
                for ( int i = 0; i < displable->points().count(); i++) {
                    if (box.isInside(displable->points().get(i))) continue;
                    cancel = true;
                }

                if (cancel) continue;

                displable->isSelectedSet(!displable->isSelected());

                displable->modifiedSet_();
            }

            cmdpts_.clean();
        }
        else {
            cmdpts_.add(pt);
        }
    }
    else {
        GFile* activeFile = ActiveFile::activeGet()->file();
        FilterFile filterFile(activeFile);

        foundFill(pt, view, &filterFile, CAD_SETTINGS.unselectMode());
        lastSelected_ = foundSelectFirst();
    }


}

//===================================================================
void Cmd_object_select::enterReset ( T2l::Display& /*view*/ )
{
    UpdateLock l;

    if (lastSelected_) {
        lastSelected_->isSelectedSet(!lastSelected_->isSelected());
    }

    lastSelected_ = foundSelectFirst();
}

//===================================================================
void Cmd_object_select::enterMove( const Point2F& pt, Display& view )
{
    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();

    if ( (cmdpts_.count() > 0 ) &&
         (CAD_SETTINGS.selectMode() == CadSettings::SEL_RECT) ) {
        Box2F box;

        box.inflateTo(pt);
        box.inflateTo(cmdpts_.get(0));

        EntityLine* line = new EntityLine();
        for ( int i = 0; i < 5; i++ ) {
            const Point2F p = box.getPoint(i);
            line->points().points().add(p);
        }

        pack->addDynamic(line);
    }

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_object_select::dialogTml() const
{
    QString result;

    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();

    // mode text ==========================================
    QString select_by = "single:;;";
    QString select_unselect = "select ";
    if (CAD_SETTINGS.selectMode() == CadSettings::SEL_RECT) {
        select_by = "by rectangle:;;";
    }
    if (CAD_SETTINGS.unselectMode()) {
        select_unselect = "unselect ";
    }
    result += "TC;CT;text: " + select_unselect + select_by;

    // mode icons -----------------------------------------
    result += "TC;CB;icon: ";
    if (CAD_SETTINGS.selectMode() == CadSettings::SEL_RECT) {
        result += dir.path() + "/resource/icons/cad_set_select_mode_rectangle.png;";
    }
    else {
        result += dir.path() + "/resource/icons/cad_set_select_mode_single.png;";
    }
    result += "cmd: cad_set_select_mode;;";

    result += "TC;CB;icon: ";
    if (CAD_SETTINGS.unselectMode()) {
        result += dir.path() + "/resource/icons/cad_set_unselect_mode_unselect.png;";
    }
    else {
        result += dir.path() + "/resource/icons/cad_set_unselect_mode_select.png;";
    }
    result += "cmd: cad_set_unselect_mode;;";

    // unselect all =======================================
    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: unselect all:;;";
    result += "TC;CB;icon: ";
    result += dir.path() + "/resource/icons/cad_unselect_all.png;";
    result += "cmd: cad_unselect_all;;";

    //=====================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace("RETANGLE", "RECTANGLE");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
QString Cmd_object_select::hint(void) const
{
    RefColSelection& selected = GObjectPool::instance().selected();

    QString result;
    QTextStream stream(&result);

    QString un = "";
    if (CAD_SETTINGS.unselectMode()) un = "un-";

    if ( CAD_SETTINGS.selectMode() == CadSettings::SEL_RECT)
    {
        QString pointNo = "first";
        if ( cmdpts_.count() > 0 ) pointNo = "second";

        stream << "enter " << pointNo << " point of selection rectangle";
    }
    else {
        if (lastSelected_ == nullptr ) {
            stream << "enter point to select object";
        }
        else {
            stream << "enter point to select/unselect object, "
                      "reset unselects and if possible it select another possible object";
        }
    }

    stream << " [" << QString::number(selected.count()) << " selected ]" ;

    /*stream << "last selected is ";
    if (lastSelected_ == nullptr ) {
        stream << "null";
    }
    else {
        stream << "NECO";
    }
    stream << " count " << this->foundSelectedCount();*/


    return result;
}

//===================================================================
