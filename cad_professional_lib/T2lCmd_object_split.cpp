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
#include "T2lCmd_object_split.h"
#include <T2lScene.h>
#include <T2lUpdateLock.h>
#include <T2lRef.h>
#include <T2lDisplay.h>
#include <T2lRefColSelection.h>
#include <T2lGObject.h>
#include <T2lGObjectPool.h>
#include "T2lObjectDisplable.h"
#include "T2lPoint2.h"
#include "T2lCadLine.h"
#include "T2lEntityLine.h"
#include "T2lCadSettings.h"
#include "T2lCadSettings2.h"
#include "T2lRay2.h"
#include "T2lFilterCadObject.h"
#include "T2lFilterCol.h"
#include "T2lFilterFile.h"
#include "T2lObPointXy.h"
#include "T2lCadObject_linePro.h"

#include <QDir>
#include <QCoreApplication>

#include <iostream>

using namespace T2l;

//===================================================================
Cmd_object_split::Cmd_object_split(void) :
    CmdCad("split")
{
}

//===================================================================
Cmd_object_split::~Cmd_object_split(void)
{
}

//===================================================================
CadObject_linePro* Cmd_object_split::getLine() const
{
    RefColSelection& selected = GObjectPool::instance().selected();

    if ( selected.count() == 0 ) return nullptr;

    GObject*         object = selected.get(0)->object();
    CadObject_linePro*  objLine   = dynamic_cast<CadObject_linePro*>(object);

    return objLine;
}

//===================================================================
void Cmd_object_split::enterReset( Display& view )
{
    UpdateLock l;

    RefColSelection& selected = GObjectPool::instance().selected();
    selected.unselectAll();

    /*if (foundSelectedCount() > 0) {
        CadObject_linePro* line = dynamic_cast<CadObject_linePro*>(foundSelectFirst());
        identifyEndpoint_(line);
    }*/

    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }

    pack->cleanDynamic();
    pack->dynamicRefresh();
}

//===================================================================
/*void Cmd_object_split::identifyEndpoint_(CadObject_linePro* line)
{
    if (line == nullptr) return;

    Point2F p0 = line->points().get(0);
    Point2F p1 = line->points().get(1);

    cadLineEnd_ = 1;
    if ( Vector2F(cmdpts_.get(0), p0).getLengthSq() < Vector2F(cmdpts_.get(0), p1).getLengthSq() ) {
        cadLineEnd_ = 0;
    }
}*/

Point2F getAssoc(const Point2F& p0, const Point2F& p1, const Point2F& p0i, const Point2F& p1i)
{
    Point2F result;

    Ray2 ray(p0, p1);
    std::cout << ray.nearestParam(p0i) << std::endl;
    if ( ray.nearestParam(p1i) <= 0 ) {
        result = p0i;
    }
    else if ( Vector2F(p0, p0i).getLengthSq() < Vector2F(p0, p1i).getLengthSq()) {
        result = p0i;
    }
    else {
        result = p1i;
    }


    return ray.getPoint(ray.nearestParam(result));
}

//===================================================================
void Cmd_object_split::enterPoint( const Point2F& pt, Display& view )
{	//<STEP> Searching scene.

    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    UpdateLock l;

    RefColSelection& selected = GObjectPool::instance().selected();

    pack->cleanDynamic();

    GFile* activeFile = ActiveFile::activeGet()->file();

    if ( getLine() == nullptr ) {
        FilterCol filterCol(FilterCol::FT_AND);
        FilterFile filterFile(activeFile);
        filterCol.add(&filterFile);
        FilterCadObject filter(FilterCadObject::ECO_LINE);
        filterCol.add(&filter);

        foundFill(pt, view, &filterCol);

        CadObject_linePro* line = dynamic_cast<CadObject_linePro*>(foundSelectFirst());
        cmdpts_.add(pt);

        //identifyEndpoint_(line);
    }
    else
    {
        CadObject_linePro* line = getLine();

        Point2F p0 = activeFile->transfGlobal2Local(line->points().get(0));
        Point2F p1 = activeFile->transfGlobal2Local(line->points().get(1));

        ObPoint& obpt = getLine()->points().getRaw(1);
        ObPointXy* xy = dynamic_cast<ObPointXy*>(&obpt);
        if (xy != nullptr) {
            Point2F ptNew = getAssoc(p0, p1, activeFile->transfGlobal2Local(cmdpts_.get(0)),
                                             activeFile->transfGlobal2Local(pt) );
            xy->move(Vector2F(xy->xy(), ptNew));
            registerFileChange();
        }

        Point2FCol ptsNew;
        ptsNew.add(p1);
        ptsNew.add(getAssoc(p1, p0, activeFile->transfGlobal2Local(cmdpts_.get(0)),
                                    activeFile->transfGlobal2Local(pt) ));
        CadObject_linePro* lineo = new CadObject_linePro(ptsNew, ActiveFile::activeGet()->file(), 0, line->style().c_str());
        lineo->colorSet(line->color());
        lineo->widthSet(line->width());
        registerFileChange();


        selected.unselectAll();
        cmdpts_.clean();
    }

    pack->dynamicRefresh();
}

//===================================================================
void Cmd_object_split::enterMove( const Point2F& pt, Display& view )
{
    ObjectDisplable* lineOb = getLine();
    //if (lineOb == nullptr) lineOb = line_;
    if (lineOb == nullptr) return;

    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }

    pack->cleanDynamic();

    Point2F pt0 = lineOb->points().get(0);
    Point2F pt1 = lineOb->points().get(1);

    EntityLine* line0 = new EntityLine( Color(255, 0, 255), 2 );
    line0->points().points().add(pt0);
    Point2F pt02 = getAssoc(pt0, pt1, cmdpts_.get(0), pt);
    line0->points().points().add(pt02);
    pack->addDynamic(line0);

    EntityLine* line1 = new EntityLine( Color(255, 0, 255), 2 );
    line1->points().points().add(pt1);
    Point2F pt12 = getAssoc(pt1, pt0, cmdpts_.get(0), pt);
    line1->points().points().add(pt12);
    pack->addDynamic(line1);

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_object_split::dialogTml() const
{
    QString result;

    //===================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
QString Cmd_object_split::hint(void) const
{
    if ( getLine() == nullptr ) {
        return "enter point to select line + forst point of splitting";
    }

    return "enter point where to split the line";
}

//===================================================================
