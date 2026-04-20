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
#include "T2lCmd_draw_linePro.h"
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

#include "T2lCadObject_linePro.h"
#include "T2lEntityLine.h"
#include "T2lPoint2.h"
#include "T2lActiveFile.h"
#include "T2lCadSettings.h"
#include "T2lCadAttr_settings.h"
#include "T2lGFile.h"
#include "T2lCadSettings2.h"
#include <QDir>
#include <QCoreApplication>

#include "T2lCadSettingsPro.h"

#include <iostream>
#include <assert.h>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_draw_linePro::Cmd_draw_linePro(void) :
    CmdCad("draw line"),
    gid0_(0)
{
}

//===================================================================
Cmd_draw_linePro::~Cmd_draw_linePro(void)
{
}

//===================================================================
void Cmd_draw_linePro::enterPoint( const Point2F& pt, Display& view )
{	UpdateLock l;

    GFile* file = ActiveFile::activeGet()->file();
    if ( file == nullptr ) return;

    int glue = false;
    if ( CAD_SETTINGS.grid() < 0 ) glue = true;

    if (points_.count() == 0) {
        gid0_ = 0;
        if ( glue ) {
            TentativeImplementationCad* tent = dynamic_cast<TentativeImplementationCad*>(CmdQueue::queue().tentative_);
            gid0_   = tent->gid_;
            index0_ = tent->index_;
        }

        points_.add(Point2<double>(pt.x(), pt.y()));
    }
    else {
        int gid1   = 0;
        int index1 = 0;

        if ( glue ) {
            TentativeImplementationCad* tent = dynamic_cast<TentativeImplementationCad*>(CmdQueue::queue().tentative_);
            gid1   = tent->gid_;
            index1 = tent->index_;
        }

        points_.add(recalculateOrtho_(pt));

        CadObject_linePro* line = new CadObject_linePro(Point2FCol(), file, 0, ATTR_SETTINGS_STYLE.get().c_str());
        registerFileChange();

        Point2F pt0 = file->transfGlobal2Local(points_.get(0));
        Point2F pt1 = file->transfGlobal2Local(points_.get(1));

        if (gid0_ <= 0) {
            line->points().append(ObPointXy(pt0));
            cout << "point0 is real";
        }
        else {
            line->points().append(ObPointRel(gid0_, index0_, pt0));
            cout << "point0 is snap";
        }

        if (gid1 <= 0) {
            line->points().append(ObPointXy(pt1));
            cout << " - point1 is real\n";
        }
        else {
            line->points().append(ObPointRel(gid1, index1, pt1));
            cout << " - point1 is snap\n";
        }

        line->colorSet(ATTR_SETTINGS_COLOR.get());
        line->widthSet(ATTR_SETTINGS_WIDTH.get());
        points_.clean();
    }

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
void Cmd_draw_linePro::enterReset( Display& view )
{
    points_.clean();

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
void Cmd_draw_linePro::enterMove( const Point2F& pt, Display& view )
{   
    if (points_.count() < 1) return;

    GFile* file = ActiveFile::activeGet()->file();
    if ( file == nullptr ) return;

    //<STEP> Searching scene.

    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;
	
    //<STEP> Dynamic drawing
    pack->cleanDynamic();
	
    Style* style = new Style("");
    Lstyle* lstyle = new Lstyle();
    lstyle->items().add(new LstyleItem(5, false));
    lstyle->items().add(new LstyleItem(4, true));
    SfeatLine* sfLine = new SfeatLine( Color::BLACK, 0.25, lstyle );
    style->sfeats().add(sfLine);

    EntityLine* line = new EntityLine( *Style::createLineStyleStr(ATTR_SETTINGS_COLOR.get(),
                          ATTR_SETTINGS_WIDTH.get(), ATTR_SETTINGS_STYLE.get().c_str()), true );

    line->points().points().add( points_.get(0) );
    line->points().points().add( recalculateOrtho_(pt) );
    pack->addDynamic(line);

    pack->dynamicRefresh();
}

//===================================================================
Point2F Cmd_draw_linePro::recalculateOrtho_( const Point2F& pt )
{
    if ( points_.count() < 1 )           return pt;

    Point2F prev = points_.get(0);
    Point2F result = pt;

    if ( CAD_SETTINGS.ortho() ) {
        Vector2F delta(prev, pt);
        if ( fabs(delta.x()) > fabs(delta.y()) ) {
            delta.set(1, 0);
        }
        else {
            delta.set(0, 0);
        }

        result = prev;
        result.add(delta);
    }

    if (CAD_SETTINGS_PARAMS.drawLengthUse().get() && CAD_SETTINGS_PARAMS.drawLength().get() != 0) {
        Vector2F line(prev, result);
        line.setLength(CAD_SETTINGS_PARAMS.drawLength().get());
        //line.multiply(2);

        result = prev;
        result.add(line);
    }

    if (CAD_SETTINGS_PARAMS.drawAngleUse().get()) {
        Vector2F line(prev, result);
        //Vector2F line2(AngleXcc(CAD_SETTINGS.draw_angle()), line.getLength());
        Vector2F line2(AngleXcc(CAD_SETTINGS_PARAMS.drawAngle().get()), line.getLength());

        result = prev;
        result.add(line2);
    }

    return result;
}

//===================================================================
QString Cmd_draw_linePro::dialogTml() const
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();

    QString result;

    /*result += "TC;CB;icon: ";
    result += "qrc:/cad_icons/resource/icons/cad_set_draw_length_use.png;";
    result += "cmd: cad_set_draw_length_use ";
    result += CAD_SETTINGS_PARAMS.drawLengthUse().get() ? "false;;":"true;;";

    if (CAD_SETTINGS.fence_size_use()) {
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS.fence_size()) + ";";
        result += "cmd: cad_set_fence_size \"$TEXT\";";
        result += "width: 40;;";
    }*/


    //===================================================
    // force length
    result += "TC;CB;icon: ";
    result += "qrc:/cad_icons/resource/icons/cad_set_draw_length_use.png;";
    result += "cmd: cad_set_draw_length_use ";
    result += CAD_SETTINGS_PARAMS.drawLengthUse().get() ? "false;;":"true;;";
    if (CAD_SETTINGS_PARAMS.drawLengthUse().get()) {
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS_PARAMS.drawLength().get()) + ";";
        result += "cmd: cad_set_draw_length \"$TEXT\";";
        result += "width: 40;;";
    }

    // force angle
    result += "TC;CB;icon: ";
    result += "qrc:/cad_icons/resource/icons/cad_set_draw_angle_use.png;";
    result += "cmd: cad_set_draw_angle_use ";
    result += CAD_SETTINGS_PARAMS.drawAngleUse().get()? "false;;":"true;;";
    if (CAD_SETTINGS_PARAMS.drawAngleUse().get()) {
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS_PARAMS.drawAngle().get()) + ";";
        result += "cmd: cad_set_draw_angle \"$TEXT\";";
        result += "width: 40;;";
    }

    //==========================================================
    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: <hup>;;";
    result += QString::fromStdString(CadAttr_dialogs::editor_color(""));

    //==========================================================
    result += "TC;CT;text: <hup>;;";
    result += QString::fromStdString(CadAttr_dialogs::editor_width());

    //==========================================================
    result += "TC;CT;text: <hup>;;";
    result += QString::fromStdString(CadAttr_dialogs::editor_lineStyles());

    //===================================================
    //REPLACING SYMBOLS
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
QString Cmd_draw_linePro::hint(void) const
{
    if (points_.count() == 0) {
        return "enter first point";
    }

    return "enter second point or reset";
}

//===================================================================
