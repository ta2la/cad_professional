//
// Copyright (C) 2022 Petr Talla. [petr.talla@gmail.com]
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
#include "T2lCmd_image_properties.h"
#include <T2lScene.h>
#include <T2lUpdateLock.h>
#include <T2lRef.h>
#include <T2lDisplay.h>
#include "T2lGObject.h"
#include "T2lCadLine.h"
#include "T2lCadObject_image.h"
#include "T2lCadSettings.h"
#include "T2lCadAttr_dialogs.h"
#include "T2lCadAttr_settings.h"
#include "T2lGObjectPool.h"
#include "T2lRefColSelection.h"
#include "T2lFilterCadObject.h"
#include "T2lActiveFile.h"
#include "T2lFilterCol.h"
#include "T2lFilterFile.h"

#include <QDir>
#include <QCoreApplication>

#include <vector>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_image_properties::Cmd_image_properties(void) :
    CmdCad("set image properties")
{
}

//===================================================================
Cmd_image_properties::~Cmd_image_properties(void)
{
}

//===================================================================
void Cmd_image_properties::enterPoint( const Point2F& pt, T2l::Display& view )
{
    T2l::EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    RefColSelection& selected = GObjectPool::instance().selected();
	
    UpdateLock l;

    if (selected.count() > 0) {
        CadObject_image* cadImage = dynamic_cast<CadObject_image*>(selected.get(0)->object());
        if (cadImage != nullptr) {
            cadImage->setTransparency(ATTR_SETTINGS_TRANSP.get());
            cadImage->setColorize(ATTR_SETTINGS_COLOR_USE.get(), ATTR_SETTINGS_COLOR.get());
            registerFileChange();
            selected.unselectAll();
        }
    }
    else {
        FilterCol filterCol(FilterCol::FT_AND);
        GFile* activeFile = ActiveFile::activeGet()->file();
        FilterFile filterFile(activeFile);
        filterCol.add(&filterFile);
        FilterCadObject filter(FilterCadObject::ECO_IMAGE);
        filterCol.add(&filter);
        foundFill(pt, view, &filterCol);
        foundSelectFirst();
    }
}

//===================================================================
void Cmd_image_properties::enterReset ( T2l::Display& view )
{
    UpdateLock l;

    RefColSelection& selected = GObjectPool::instance().selected();
    selected.unselectAll();

    if (foundSelectedCount() >= 0) {
        foundSelectFirst();
    }
}

//===================================================================
QString Cmd_image_properties::dialogTml() const
{
    QString result;

    result += QString::fromStdString(CadAttr_dialogs::editor_transparency());

    result += "TC;CT;text: <hup>;;";
    if (! ATTR_SETTINGS_COLOR_USE.get()) {
        result += "TC;CB;text: colorize not used;cmd: cad_set_color_use;;";
    }
    else {
        result += "TC;CT;text: colorize:;;TC;CT;text:<space>;;";
        result += QString::fromStdString(CadAttr_dialogs::editor_color(""));
        result += "TC;CT;text: <space>;;";
        result += "TC;CB;text: not use;cmd: cad_set_color_use;;";
    }

    //===================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
QString Cmd_image_properties::hint(void) const
{
    RefColSelection& selected = GObjectPool::instance().selected();

    if (selected.count() == 0) {
        return "enter point to select image";    }

    return "enter point to apply image properties";
}

//===================================================================
