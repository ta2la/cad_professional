//
// Copyright (C) 2020 Petr Talla. [petr.talla@gmail.com]
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
#include "T2lCmd_draw_arc2.h"
#include <T2lUpdateLock.h>
#include <T2lScene.h>
#include <T2lDisplay.h>
#include "T2lStyle.h"
#include "T2lSfeatLine.h"
#include "T2lCmdQueue.h"
#include "T2lTentativeImplementationCad.h"
#include "T2lObPointXy.h"
#include "T2lObPointRel.h"

#include "T2lCadAttr_dialogs.h"

#include "T2lLstyle.h"
#include "T2lStyle.h"

#include "T2lCadObject_arc.h"
#include "T2lCadObject_linePro.h"
#include "T2lEntityLine.h"
#include "T2lEntityText.h"
#include "T2lPoint2.h"
#include "T2lActiveFile.h"
#include "T2lCadSettings.h"
#include "T2lCadAttr_settings.h"
#include "T2lGFile.h"
#include <QDir>
#include <QCoreApplication>

#include "T2lCadSettingsPro.h"

#include <iostream>
#include <assert.h>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_draw_arc2::Cmd_draw_arc2(void) :
    CmdCad("draw arc"),
    gid0_(0)
{
}

//===================================================================
Cmd_draw_arc2::~Cmd_draw_arc2(void)
{
}

//===================================================================
void Cmd_draw_arc2::enterPoint( const Point2F& pt, Display& view )
{

    GFile* file = ActiveFile::activeGet()->file();
    if ( file == nullptr ) return;

    if (points_.count() < 2) {
        points_.add(Point2<double>(pt.x(), pt.y()));
    }
    else {
        points_.add(pt);

        UpdateLock l;

        Point2F pt0 = file->transfGlobal2Local(points_.get(0));
        Point2F pt1 = file->transfGlobal2Local(points_.get(1));
        Point2F pt2 = file->transfGlobal2Local(points_.get(2));

        /*if (CadObject_arc::isCcWise(pt0, pt1, Vector2F(pt1, pt2))) {
            pt0 = pt2;
            pt2 = points_.get((0));
        }*/

        Point2FCol pts;

        pts.add(pt0);
        pts.add(pt1);
        pts.add(pt2);

        CadObject_arc* line = new CadObject_arc(pts, file, 0, ATTR_SETTINGS_STYLE.get().c_str());
        registerFileChange();

        line->colorSet(ATTR_SETTINGS_COLOR.get());
        line->widthSet(ATTR_SETTINGS_WIDTH.get());

        points_.clean();
    }

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
void Cmd_draw_arc2::enterReset( Display& view )
{
    points_.clean();

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
void Cmd_draw_arc2::enterMove( const Point2F& pt, Display& view )
{
    //<STEP> synamic drawing prereqisities
    if (points_.count() < 1) return;

    GFile* file = ActiveFile::activeGet()->file();
    if ( file == nullptr ) return;

    //<STEP> Searching scene.
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    //<STEP> draw dynamic
    pack->cleanDynamic();

    Point2F p0 = points_.get(0);

    EntityLine* line0 = new EntityLine( *Style::createLineStyleStr(Color::BLACK,
                              0.4, ATTR_SETTINGS_STYLE.get().c_str()), true );

    if ( points_.count() > 1 && ( p0.x()!=pt.x() || p0.y()!=pt.y() ) ) {
        Point2F xy;
        double r;

        Point2F p1 = points_.get(1);
        Point2F p2 = pt;

        CadObject_arc::normalize(p0, p1, p2);

        EntityLine* line = new EntityLine( *Style::createLineStyleStr(ATTR_SETTINGS_COLOR.get(),
                          0.18, ATTR_SETTINGS_STYLE.get().c_str()), true );
        CadObject_arc::pointsCurve(line->points().points(), p0, p1, p2);
        pack->addDynamic(line);

        Color::EPredef color = Color::ORANGE;

        line0->points().points().add( p0 );
        line0->points().points().add( p1 );

        EntityLine* line2 = new EntityLine( *Style::createLineStyleStr( color,
                                  2, ATTR_SETTINGS_STYLE.get().c_str()), true );
        line2->points().points().add( p1 );
        line2->points().points().add( p2 );
        pack->addDynamic(line2);
    }
    else {
        line0->points().points().add( p0 );
        line0->points().points().add( pt );
    }

    pack->addDynamic(line0);

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_draw_arc2::dialogTml() const
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();

    QString result;

    //===================================================
    // force length
    /*result += "TC;CB;icon: ";
    result += "qrc:/cad_icons/resource/icons/cad_set_draw_length_use.png;";
    result += "cmd: cad_set_draw_length_use;;";
    if (CAD_SETTINGS.draw_length_use()) {
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS.draw_length()) + ";";
        result += "cmd: cad_set_draw_length \"$TEXT\";";
        result += "width: 40;;";
    }*/

    // force angle
    /*result += "TC;CB;icon: ";
    result += "qrc:/cad_icons/resource/icons/cad_set_draw_angle_use.png;";
    result += "cmd: cad_set_draw_angle_use;;";
    if (CAD_SETTINGS.draw_angle_use()) {
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS.draw_angle()) + ";";
        result += "cmd: cad_set_draw_angle \"$TEXT\";";
        result += "width: 40;;";
    }*/

    //==========================================================
    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: <hup>;;";
    result += QString::fromStdString(CadAttr_dialogs::editor_color(""));

    //==========================================================
    result += "TC;CT;text: <hup>;;";
    result += QString::fromStdString(CadAttr_dialogs::editor_width());

    //==========================================================
    result += "TC;CT;text: <hup>;;";
    result +=  QString::fromStdString(CadAttr_dialogs::editor_lineStyles());

    //===================================================
    //REPLACING SYMBOLS
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
QString Cmd_draw_arc2::hint(void) const
{
    if (points_.count() == 0) {
        return "enter first point";
    }

    return "enter second point or reset";
}

//===================================================================
