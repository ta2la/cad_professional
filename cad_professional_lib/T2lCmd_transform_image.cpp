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
#include "T2lCmd_transform_image.h"
#include <T2lScene.h>
#include "T2lFilterCadObject.h"
#include "T2lFilterCol.h"
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
#include "T2lObPointXy.h"

using namespace T2l;

//===================================================================
Cmd_transform_image::Cmd_transform_image(void) :
    CmdCad("move"),
    previousDefined_(false)
{
}

//===================================================================
Cmd_transform_image::~Cmd_transform_image(void)
{
}

//===================================================================
void Cmd_transform_image::enterPoint( const Point2F& pt, Display& view )
{
    if ( ActiveFile::activeGet() == nullptr ) return;

    //<STEP> Searching scene.
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    UpdateLock l;

    if ( GObjectPool::instance().selected().count() == 0 )
    {
        foundClean();

        GFile* activeFile = ActiveFile::activeGet()->file();
        FilterCol filterCol(FilterCol::FT_AND);
        FilterFile filterFile(activeFile);
        filterCol.add(&filterFile);
        FilterCadObject filter(FilterCadObject::ECO_LINE);
        filterCol.add(&filter);

        foundFill(pt, view, &filterFile);
        foundSelectFirst();
    }
    else
    {
        if ( cmdpts_.count() < 4) {
            cmdpts_.add(pt);
        }
        else {
            Point2F p0 = cmdpts_.get(0);
            Point2F p1 = cmdpts_.get(1);
            Point2F p2 = cmdpts_.get(2);
            Point2F p3 = cmdpts_.get(3);

            double sx = (p1.x()-p3.x())/(p0.x()-p2.x());
            double dx = p1.x()-p0.x()*sx;

            double sy = (p1.y()-p3.y())/(p0.y()-p2.y());
            double dy = p1.y()-p0.y()*sy;

            RefColSelection& selected = GObjectPool::instance().selected();
            TcObject* object = selected.get(0)->object();
            CadObject_image* image = dynamic_cast<CadObject_image*>(object);
            if (image == nullptr) {
                selected.unselectAll();
                cmdpts_.clean();
            }

            ObPointXy* xy0 = dynamic_cast<ObPointXy*>(&image->points().getRaw(0));
            ObPointXy* xy1 = dynamic_cast<ObPointXy*>(&image->points().getRaw(1));

            if (xy0==nullptr || xy1==nullptr) {
                selected.unselectAll();
                cmdpts_.clean();
            }

            if (image->points().count()>1) {
                Point2F P0 = image->points().get(0);
                Point2F P1 = image->points().get(1);

                double x0 = P0.x()*sx + dx;
                double y0 = P0.y()*sy + dy;

                double x1 = P1.x()*sx + dx;
                double y1 = P1.y()*sy + dy;

                xy0->move(Vector2F(xy0->xy(), Point2F(x0, y0) ));
                xy1->move(Vector2F(xy1->xy(), Point2F(x1, y1) ));
            }

            image->modifiedSet_();

            registerFileChange();

            selected.unselectAll();
            foundClean();
            cmdpts_.clean();
        }
    }

    //selected.unselectAll();
    //foundClean();
    //previousDefined_ = false;
}

//===================================================================
void Cmd_transform_image::enterReset ( T2l::Display& view )
{
    UpdateLock l;

    RefColSelection& selected = GObjectPool::instance().selected();
    selected.unselectAll();

    if (foundSelectedCount() == 0) {
        previousDefined_ = false;
    }
    else {
        foundSelectFirst();
    }
}

//===================================================================
void Cmd_transform_image::enterMove( const Point2F& pt, Display& view )
{
    //<STEP> Searching scene.
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    //<STEP> Dynamic drawing
    pack->cleanDynamic();

    if (cmdpts_.count() > 0) {
        Point2F P0 = cmdpts_.get(0);
        Point2F P1 = pt;
        if (cmdpts_.count() > 1) {
            P1 = cmdpts_.get(1);
        }

        EntityLine* line = new EntityLine( Color::MAGENTA, 0.5 );
        line->points().points().add( P0 );
        line->points().points().add( P1 );
        pack->addDynamic(line);

        if (cmdpts_.count() > 2) {
            P0 = cmdpts_.get(2);
            P1 = pt;
            if (cmdpts_.count() > 3) {
                P1 = cmdpts_.get(3);
            }
        }

        line = new EntityLine( Color::MAGENTA, 0.5 );
        line->points().points().add( P0 );
        line->points().points().add( P1 );
        pack->addDynamic(line);

        /*GFile* file = ActiveFile::activeGet()->file();
        if ( file != nullptr ) { //only active file content can be changed
            P0 = file->transfGlobal2Local(P0);
            P1 = file->transfGlobal2Local(P1);
        }*/

        //RefColSelection& selected = GObjectPool::instance().selected();

        //if ( selected.count() == 0 ) return;
    }

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_transform_image::hint(void) const
{
    if (SELECTED_COL.count() < 1) {
        return "select image to transform";
    }
    else {
        if (cmdpts_.count()==0 || cmdpts_.count()==2) {
            return "enter point on image, to be transformed";
        }
        if (cmdpts_.count()==1 || cmdpts_.count()==3) {
            return "enter point where to transform image point";
        }
    }

    return "enter point to confirm transformation";
}

//===================================================================
