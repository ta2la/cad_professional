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
#include "T2lCmd_active_file_move.h"
#include <T2lUpdateLock.h>
#include <T2lScene.h>
#include <T2lDisplay.h>
#include "T2lStyle.h"
#include "T2lCadLine.h"
#include "T2lEntityLine.h"
#include "T2lPoint2.h"
#include "T2lActiveFile.h"
#include "T2lCadSettings.h"
#include "T2lGFile.h"
#include "T2lCadObject_text.h"
#include "T2lEntityText.h"

#include <iostream>
#include <assert.h>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_active_file_move::Cmd_active_file_move(void) :
    Cmd("move active file"),
    r_(1)
{
}

//===================================================================
Cmd_active_file_move::~Cmd_active_file_move(void)
{
}

//===================================================================
void Cmd_active_file_move::enterPoint( const Point2F& pt, Display& view )
{
    UpdateLock l;

    if (CAD_SETTINGS.referenceTransfMode() != CadSettings::RSM_MOVE) return;

    if (points_.count() < 1) {
        points_.add(Point2<double>(pt.x(), pt.y()));
    }
    else {
        GFile* file = ActiveFile::activeGet()->file();

        if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_MOVE ) {
            Vector2F offset(points_.get(0), recalculateOrtho_(pt));
            offset.add(file->getOffset());
            file->setOffset(offset);
        }

        file->refresh();
        points_.clean();
    }

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
void Cmd_active_file_move::enterMove( const Point2F& pt, Display& view )
{   
    if (CAD_SETTINGS.referenceTransfMode() != CadSettings::RSM_MOVE) return;
    if (points_.count() < 1) return;

    //<STEP> Searching scene.

    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;
	
    //<STEP> Dynamic drawing
    pack->cleanDynamic();
	
    EntityLine* line = new EntityLine( Color::MAGENTA, 1 );
    line->points().points().add( points_.get(0) );
    Point2F p = pt;
    if ( CAD_SETTINGS.referenceTransfMode() == false) {
        p = recalculateOrtho_(p);
    }
    line->points().points().add(p);
    pack->addDynamic(line);

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_active_file_move::hint(void) const
{
    if (points_.count() < 1) {
        return "enter point as origin of the movement";
    }
    else {
        return "enter point as end of movement";
    }
}

//===================================================================
Point2F Cmd_active_file_move::recalculateOrtho_( const Point2F& pt )
{
    if ( points_.count() < 1 )           return pt;
    if ( CAD_SETTINGS.ortho() == false ) return pt;

    Point2F prev = points_.get(0);

    Vector2F delta(prev, pt);
    if ( fabs(delta.x()) > fabs(delta.y()) ) {
        delta.set(1, 0);
    }
    else {
        delta.set(0, 0);
    }

    Point2F result = prev;
    result.add(delta);

    return result;
}

QString Cmd_active_file_move::dialogTml() const
{
    QString result;

    GFile* file = ActiveFile::activeGet()->file();
    if (file == nullptr)

    result += "TC;CT;text: active file: ";
    result += file->fileName();
    result += ";;";

    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: offset xy:;;";
    result += "TC;CT;text: <space>;;";
    result += "TC;control: spacer;spacer: minx;value: 60;;";
    result += "TC;control: edit;minx: 500;";
    result += "text: ";
    result += QString::number(file->getOffset().x());
    result += ";width: 80;";
    result += "cmd: cmd_cad_set_afile_x \"$TEXT\";;";
    result += "TC;control: edit;";
    result += "text: ";
    result += QString::number(file->getOffset().y());
    result += ";width: 80;";
    result += "cmd: cmd_cad_set_afile_x \"$TEXT\";;";

    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: scale:;;";
    result += "TC;CT;text: <space>;;";
    result += "TC;control: spacer;spacer: minx;value: 60;;";
    result += "TC;control: edit;";
    result += "text: ";
    result += QString::number(file->getScale());
    result += ";width: 40;";
    result += "cmd: cad_set_afile_scale \"$TEXT\";;";

    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: flip xy:;;";
    result += "TC;CT;text: <space>;;";
    result += "TC;control: spacer;spacer: minx;value: 60;;";
    result += "TC;control: edit;";
    result += "text: ";
    result += QString::number(file->getTransfAngle().get());
    result += ";width: 40;";
    result += "cmd: cad_set_afile_transf_angle \"$TEXT\";;";

    result += "TC;control: edit;";
    result += "text: ";
    result += QString::number(file->getTransfFlipX());
    result += ";width: 40;";
    result += "cmd: cad_set_afile_transf_flipx \"$TEXT\";;";

    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: rotate:;;";
    result += "TC;control: spacer;spacer: minx;value: 60;;";
    result += "TC;CT;text: <space>;;";
    result += "TC;control: edit;";
    result += "text: ";
    result += QString::number(file->getTransfFlipY());
    result += ";width: 40;";
    result += "cmd: cad_set_afile_transf_angle \"$TEXT\";;";

    //===================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
/*QString Cmd_active_file_move::dialogTml() const
{
    QString result;

    result += "TC;CT;text: active file: ";
    result += ActiveFile::activeGet()->file()->fileName();
    result += ";;";
    result += "TC;CT;text: <hup>;;";

    result += "TC;CT;text: ";
    if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_MOVE)        result += "offset;";
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_SCALE)  result += "scale;";
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_FLIPX)  result += "flip x;";
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_FLIPY)  result += "flip y;";
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_ROTATE) result += "rotate;";
    result += "cmd: cad_set_reference_scale_mode;;";

    result += "TC;CT;text: <space>;;";

    if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_MOVE) {
    }
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_SCALE) {
        GFile* file = ActiveFile::activeGet()->file();
        if (file) {
            result += "TC;control: edit;";
            result += "text: ";
            result += QString::number(file->getScale());
            result += ";width: 40;";
            result += "cmd: cad_set_afile_scale \"$TEXT\";;";
        }
    }
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_ROTATE) {
        GFile* file = ActiveFile::activeGet()->file();
        if (file) {
            result += "TC;control: edit;";
            result += "text: ";
            result += QString::number(file->getTransfAngle().get());
            result += ";width: 40;";
            result += "cmd: cad_set_afile_transf_angle \"$TEXT\";;";
        }
    }
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_FLIPX) {
        GFile* file = ActiveFile::activeGet()->file();
        if (file) {
            result += "TC;control: edit;";
            result += "text: ";
            result += QString::number(file->getTransfFlipX());
            result += ";width: 40;";
            result += "cmd: cad_set_afile_transf_flipx \"$TEXT\";;";
        }
    }
    else if ( CAD_SETTINGS.referenceTransfMode() == CadSettings::RSM_FLIPY) {
        GFile* file = ActiveFile::activeGet()->file();
        if (file) {
            result += "TC;control: edit;";
            result += "text: ";
            result += QString::number(file->getTransfFlipY());
            result += ";width: 40;";
            result += "cmd: cad_set_afile_transf_flipy \"$TEXT\";;";
        }
    }


    //===================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}*/

//===================================================================
