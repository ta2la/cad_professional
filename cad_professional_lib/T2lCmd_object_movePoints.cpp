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
#include "T2lCmd_object_movePoints.h"
#include <T2lScene.h>
#include <T2lUpdateLock.h>
#include <T2lRef.h>
#include <T2lDisplay.h>
#include <T2lRefColSelection.h>
#include <T2lGObject.h>
#include <T2lGObjectPool.h>
#include "T2lObjectDisplable.h"
#include "T2lPoint2.h"
#include "T2lEntityLine.h"
#include "T2lActiveFile.h"
#include "T2lObPointXy.h"
#include "T2lGFile.h"
#include "T2lCadSettings.h"

#include <QDir>
#include <QCoreApplication>

#include "iostream"

using namespace T2l;
using namespace std;

//===================================================================
Cmd_object_movePoints::Cmd_object_movePoints(void) :
    CmdCad("move vertices")
{
}

//===================================================================
Cmd_object_movePoints::~Cmd_object_movePoints(void)
{
}

//===================================================================
void Cmd_object_movePoints::enterPoint( const Point2F& pt, Display& view )
{
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) return;

    GFile* activeFile = ActiveFile::activeGet()->file();

    UpdateLock l;

    if ( points_.count()==3 ||
         ( points_.count()==1 &&  CAD_SETTINGS.fence_size_use() ) )
    {
        Box2F box;
        Point2F p0 = activeFile->transfGlobal2Local(points_.get(0));
        Point2F beg;

        box.inflateTo(p0);
        if (points_.count()==1) {
            box.inflateBy(CAD_SETTINGS.fence_size()/2);
            beg = p0;
        }
        else {
            Point2F p1 = activeFile->transfGlobal2Local(points_.get(1));
            box.inflateTo(p1);
            beg = activeFile->transfGlobal2Local(points_.get(2));
        }

        Point2F pt2 = activeFile->transfGlobal2Local(pt);

        //cout << "box: " << box.x().beg() << "-" << box.x().end() << "  " << box.x().beg() << "-" << box.x().end() << endl;

        Vector2F delta( beg, pt2 );

        for ( long i = 0; i < pack->scene()->count(); i++ )
        {
            Ref* ref = pack->scene()->get(i);
            ObjectDisplable* displable = dynamic_cast<ObjectDisplable*>(ref->object());
            if (displable == nullptr) continue;
            if (displable->parent_ == nullptr) continue;

            if ( activeFile != displable->parent_) continue;

            bool changed = false;

            for ( int p = 0; p < displable->points_.count(); p++ ) {
                Point2F pti = displable->points().getRaw(p).xy();
                if (box.isInside(pti) == false) continue;

                Point2F pt2 = activeFile->transfGlobal2Local(pti);
                Vector2F d(pti, pt2);
                d.add(delta);
                displable->points().getRaw(p).move(delta);

                changed = true;

                /*Vector2F delta( xy->xy(), pt2 );
                if (xy == nullptr) continue;
                if ( box.isInside(xy->xy()) == false) continue;
                xy->move(delta);*/


                /*Point2F pti = clone->points().getRaw(i).xy();
                Point2F pt2 = file->transfGlobal2Local(pti);
                Vector2F d(pti, pt2);
                d.add(delta);
                clone->points().getRaw(i).move(d);*/
            }

            if (changed) registerFileChange();

            displable->modifiedSet_();
        }

        points_.clean();
    }
    else {
        points_.add(pt);
    }

    pack->cleanDynamic();
    pack->dynamicRefresh();

    //view.refresh();
}

//===================================================================
void Cmd_object_movePoints::enterReset( Display& view )
{
    points_.clean();
}

//===================================================================
void Cmd_object_movePoints::enterMove( const Point2F& pt, Display& view )
{
    //if ( points_.count() == 0 ) return;

    EntityPack* pack = view.entityPack();
    pack->cleanDynamic();

    Box2F box;
    if (points_.count()==0 && CAD_SETTINGS.fence_size_use()) {
        box.inflateTo(pt);
        box.inflateBy(CAD_SETTINGS.fence_size()/2);
    }
    else if (points_.count()==1 && CAD_SETTINGS.fence_size_use()) {
        box.inflateTo(pt);
        box.inflateBy(CAD_SETTINGS.fence_size()/2);
    }
    else if (points_.count() == 1) {
        box.inflateTo(pt);
        box.inflateTo(points_.get(0));
    }
    else if (points_.count() == 2) {
        box.inflateTo(points_.get(0));
        box.inflateTo(points_.get(1));
    }
    else if (points_.count() == 3) {
        box.inflateTo(points_.get(0));
        box.inflateTo(points_.get(1));

        box.moveBy(Vector2F(points_.get(2), pt));
    }
    else {
        return;
    }

    EntityLine* line = new EntityLine();
    for ( int i = 0; i < 5; i++ ) {
        const Point2F p = box.getPoint(i);
        line->points().points().add(p);
    }

    pack->addDynamic(line);
    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_object_movePoints::dialogTml() const
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();

    QString result;

    //===================================================
    // force length
    result += "TC;CB;icon: ";
    result += dir.path() + "/resource/icons/cad_set_draw_length_use.png;";
    result += "cmd: cad_set_fence_size_use;;";
    if (CAD_SETTINGS.fence_size_use()) {
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS.fence_size()) + ";";
        result += "cmd: cad_set_fence_size \"$TEXT\";";
        result += "width: 40;;";
    }

    //===================================================
    //REPLACING SYMBOLS
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
QString Cmd_object_movePoints::hint(void) const
{
    if ( points_.count() == 0 ) {
        return "enter corner of fence";
    }
    else if ( points_.count() == 1 ) {
        return "enter second corner of fence";
    }
    else if ( points_.count() == 2 ) {
        return "enter first point of movement";
    }

    return "enter second point of movement";
}

//===================================================================
