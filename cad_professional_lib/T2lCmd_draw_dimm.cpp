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
#include "T2lCmd_draw_dimm.h"
#include <T2lUpdateLock.h>
#include <T2lScene.h>
#include <T2lDisplay.h>
#include "T2lStyle.h"

#include "T2lCadObject_dimm.h"
#include "T2lEntityLine.h"
#include "T2lPoint2.h"
#include "T2lActiveFile.h"
#include "T2lCadSettings.h"
#include "T2lCadAttr_settings.h"
#include "T2lCadSettings2.h"
#include "T2lCadAttr_dialogs.h"
#include "T2lGFile.h"
#include <QDir>
#include <QCoreApplication>

#include <iostream>
#include <assert.h>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_draw_dimm::Cmd_draw_dimm(void) :
    CmdCad("draw dimension")
{
}

//===================================================================
Cmd_draw_dimm::~Cmd_draw_dimm(void)
{
}

//===================================================================
void Cmd_draw_dimm::enterPoint( const Point2F& pt, Display& view )
{	UpdateLock l;

    GFile* file = ActiveFile::activeGet()->file();
    if ( file == nullptr ) return;

    if (points_.count() <= 1) {
        points_.add(pt);
    }
    else {
        points_.add(pt);

        Point2FCol pts;
        for ( int i = 0; i < points_.count(); i++) {
            pts.add( file->transfGlobal2Local(points_.get(i)) );
        }

        double dir = -1;
        if (CAD_SETTINGS_PARAMS.drawAngleUse().get()) {
            dir = AngleXcc(CAD_SETTINGS_PARAMS.drawAngle().get()).get();
        }

        CadObject_dimm* dimm = new CadObject_dimm(pts, dir, file);
        registerFileChange();

        dimm->setMetricText(true);
        dimm->sizeSet(ATTR_SETTINGS_SIZE.get()*CAD_SETTINGS.symbol_scale());

        points_.clean();
    }

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
void Cmd_draw_dimm::enterReset( Display& view )
{
    points_.clean();

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();
    pack->dynamicRefresh();
}


//===================================================================
void Cmd_draw_dimm::enterMove( const Point2F& pt, Display& view )
{   
    if (points_.count() < 1) return;

    //<STEP> Searching scene.

    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;
	
    //<STEP> Dynamic drawing
    pack->cleanDynamic();
	
    EntityLine* line = new EntityLine( Color::ORANGE, 0.5 );
    line->points().points().add( points_.get(0) );
    if (points_.count() > 1 ) {
        line->points().points().add( points_.get(1) );
    }
    else {
        line->points().points().add( pt );
    }
    pack->addDynamic(line);

    if (points_.count() > 1 ) {
        /*Ray2 ray(points_.get(0), points_.get(1));
        Point2F ptC = ray.getPoint(ray.nearestParam(pt));

        Point2F p0 = pt;
        p0.add( Vector2F(ptC, points_.get(0) ) );

        Point2F p1 = pt;
        p1.add( Vector2F(ptC, points_.get(1) ) );

        EntityLine* line2 = new EntityLine( Color::BLACK, 0.15 );
        line2->points().points().add( p0 );
        line2->points().points().add( p1 );

        pack->addDynamic(line2);*/

        EntityList list;

        Point2FCol points;
        points.add(points_.get(0));
        points.add(points_.get(1));
        points.add(pt);

        double dir = -1;
        if (CAD_SETTINGS_PARAMS.drawAngleUse().get()) {
            dir = AngleXcc(CAD_SETTINGS_PARAMS.drawAngle().get()).get();
        }

        CadObject_dimm dimm(points, dir, nullptr);
        dimm.setMetricText(true);
        dimm.sizeSet(ATTR_SETTINGS_SIZE.get()*CAD_SETTINGS.symbol_scale());

        dimm.display(list, view.entityPack()->scene());
        for ( long i = 0; i < list.count(); i++ ) {
            pack->addDynamic(list.get(i));
        }
    }

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_draw_dimm::dialogTml() const
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();

    QString result;

    //<STEP> force angle
    result += "TC;CB;icon: ";
    result += "qrc:/cad_icons/resource/icons/cad_set_draw_angle_use.png;";
    result += "cmd: cad_set_draw_angle_use ";
    result += CAD_SETTINGS_PARAMS.drawAngleUse().get() ? "false;;":"true;;";
    if (CAD_SETTINGS_PARAMS.drawAngleUse().get()) {
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS_PARAMS.drawAngle().get()) + ";";
        result += "cmd: cad_set_draw_angle \"$TEXT\";";
        result += "width: 40;;";
    }
    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: <hup>;;";

    //<STEP>
    result += QString::fromStdString(CadAttr_dialogs::editor_size(true, CAD_SETTINGS.symbol_scale()));

    //<STEP>
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
QString Cmd_draw_dimm::hint(void) const
{
    if (points_.count() == 0) {
        return "enter first point";
    }
    else if (points_.count() == 0) {
        return "enter second point or reset";
    }
    else {
        return "enter dimension position or reset";
    }
}

//===================================================================
