//
// Copyright (C) 2015 Petr Talla. [petr.talla@gmail.com]
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
#include "T2lCmd_draw_papper.h"
#include "T2lCadObject_papper.h"
#include "T2lCadObject_symbol.h"
#include <T2lUpdateLock.h>
#include <T2lScene.h>
#include <T2lDisplay.h>
#include "T2lStyle.h"
#include "T2lEntityPoint.h"
#include "T2lCadSettings.h"
#include "T2lActiveFile.h"
#include "T2lGFile.h"
#include "T2lEnPointMmRel.h"

#include <QDir>
#include <QCoreApplication>

#include <iostream>
#include <assert.h>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_draw_papper::Cmd_draw_papper(void) :
    CmdCad( "draw papper" )
{
}

//===================================================================
Cmd_draw_papper::~Cmd_draw_papper(void)
{
}

//===================================================================
void Cmd_draw_papper::enterPoint( const T2l::Point2F& pt, Display& view )
{
    UpdateLock l;

    Point2FCol points;
    points.add(pt);
    points.add(Point2F(pt.x()+10000, pt.y()));
    points.add(Point2F(pt.x()+10000, pt.y()+15000));
    points.add(Point2F(pt.x(), pt.y()+15000));

    new CadObject_papper( pt, CAD_SETTINGS.papperFactor(), CAD_SETTINGS.drawingUnitInMm(),
                          CAD_SETTINGS.papperWidthInMm(), CAD_SETTINGS.papperHeightInMm(),
                          ActiveFile::activeGet()->file(), CAD_SETTINGS.papperMaskLess() );
    registerFileChange();

    T2l::EntityPack* pack = view.entityPack();

    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
void Cmd_draw_papper::enterMove( const T2l::Point2F& pt, Display& view )
{
    //<STEP> Searching scene.
    T2l::EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    //<STEP> DYnamic drawing
    pack->cleanDynamic();

    EntityList list;
    CadObject_papper papper( pt, CAD_SETTINGS.papperFactor(), CAD_SETTINGS.drawingUnitInMm(),
                             CAD_SETTINGS.papperWidthInMm(), CAD_SETTINGS.papperHeightInMm(), nullptr,
                             CAD_SETTINGS.papperMaskLess() );
    papper.display(list, nullptr);
    for ( long i = 0; i < list.count(); i++ ) {
        pack->addDynamic(list.get(i));
    }

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_draw_papper::dialog() const {
    QString result;

    return result;
}

//===================================================================
QString Cmd_draw_papper::dialogTml() const
{
    QString result;

    result += "TC;control: text;";
    result += "text: scale:;;";
    result += "TC;CT;text: <space>;;";

    result += "TC;control: text;";
    result += "text: 1:50;";
    result += "cmd: cad_set_papper_factor 50;";
    result += "width: 40;;";
    result += "TC;CT;text: <space>;;";

    result += "TC;control: text;";
    result += "text: 1:100;";
    result += "cmd: cad_set_papper_factor 100;";
    result += "width: 40;;";
    result += "TC;CT;text: <space>;;";

    result += "TC;control: text;";
    result += "text: 1:200;";
    result += "cmd: cad_set_papper_factor 200;";
    result += "width: 40;;";
    result += "TC;CT;text: <space>;;";

    result += "TC;control: text;";
    result += "text: 1:500;";
    result += "cmd: cad_set_papper_factor 500;";
    result += "width: 40;;";
    result += "TC;CT;text: <space>;;";

    result += "TC;control: text;";
    result += "text: 1:1000;";
    result += "cmd: cad_set_papper_factor 1000;";
    result += "width: 40;;";
    result += "TC;CT;text: <space>;;";

    result += "TC;control: edit;";
    result += "text: " + QString::number(CAD_SETTINGS.papperFactor()) + ";";
    result += "cmd: cad_set_papper_factor \"$TEXT\";";
    result += "width: 40;;";

    result += "TC;CT;text: <hup>;;";
    result += "TC;control: text;";
    result += "text: drawing unit in [mm]:;;";
    result += "TC;CT;text: <space>;;";
    result += "TC;control: spacer;spacer: minx;value: 135;;";
    result += "TC;control: edit;";
    result += "text: " + QString::number(CAD_SETTINGS.drawingUnitInMm()) + ";";
    result += "cmd: cad_set_drawing_unit_in_mm \"$TEXT\";";
    result += "width: 40;;";

    result += "TC;CT;text: <hup>;;";
    result += "TC;control: text;";
    result += "text: papper width/height [mm]:;;";
    result += "TC;CT;text: <space>;;";
    result += "TC;control: spacer;spacer: minx;value: 135;;";
    result += "TC;control: edit;";
    result += "text: " + QString::number(CAD_SETTINGS.papperWidthInMm()) + ";";
    result += "cmd: cad_set_papper_width_in_mm \"$TEXT\";";
    result += "width: 40;;";
    result += "TC;control: edit;";
    result += "text: " + QString::number(CAD_SETTINGS.papperHeightInMm()) + ";";
    result += "cmd: cad_set_papper_height_in_mm \"$TEXT\";";
    result += "width: 40;;";

    result += "TC;CT;text: <hup>;;";
    result += "TC;control: text;";
    result += "text: ";
    if (CAD_SETTINGS.papperMaskLess()) {
        result += "maskless;";
    }
    else {
        result += "mask;";
    }
    result += "cmd: cad_set_papper_maskless;;";

    //===================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
