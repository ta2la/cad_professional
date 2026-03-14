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
#include "T2lCadObject_reference.h"

#include "T2lGFile.h"
#include "T2lCadLine.h"
#include <T2lStyle.h>
#include "T2lEntityList.h"
#include "T2lSfeatArea.h"
#include "T2lObPointXy.h"

#include "T2lStoredItem.h"
#include "T2lEntityLine.h"
#include "T2lEntityArea.h"
#include "T2lStoredAttrNUM.h"
#include "T2lStoredAttrSTR.h"

#include "T2lCadSettings.h"

#include <QString>

#include <iostream>
#include <sstream>

using namespace T2l;
using namespace std;

//===================================================================
CadObject_reference::CadObject_reference( const Point2Col<double>& points, const Color& color, const Color& colorLine, GFile* parent,
                                bool color_use, bool color_line_use ) :
    ObjectDisplable(points, parent),
    color_(color),
    colorLine_(colorLine),
    color_use_(color_use),
    color_line_use_(color_line_use)
{
    if (parent != nullptr) parent->add(this);
}

//=============================================================================
ObjectDisplable* CadObject_reference::clone()
{
    CadObject_reference* area = new CadObject_reference(Point2FCol(), color_, colorLine_, parent_, color_use_, color_line_use_);
    for ( int i = 0; i < points_.count(); i++ ) {
        Point2F pt = points_.get(i);
        area->points().append(ObPointXy(pt));
    }
    return area;
}

//===================================================================
CadObject_reference::~CadObject_reference(void)
{
}

//===================================================================
void CadObject_reference::display(EntityList& list, RefCol* scene)
{
   double width  = 0.25;

    //if (parent_ == nullptr) return;

    if (color_line_use_) {
        EntityLine* line = new EntityLine( colorLine_, width, nullptr );
        for ( int i = 0; i < points_.count(); i++ ) {
            Point2F pti = points_.get(i);
            //pti.add(parent()->getOffset());
            line->points().points().add( pti );
        }
        line->points().points().add( points_.get(0));
        list.add( line );
    }

    if (color_use_) {
        Style* style = new Style("");
        style->sfeats().add( new SfeatArea(color_, 110));

        EntityArea* entityArea = new EntityArea( *style, true, nullptr );
        for (int i = 0; i < points().count(); i++) {
            entityArea->points().points().add( points().get(i) );
        }

        list.add( entityArea );
    }

    displayChange_(list);
}


//=========================================================================
bool CadObject_reference::loadFromStored( StoredItem* item, GFile* file )
{
    StoredAttr* type = item->get("type");
    if (type->getAsSTR() == nullptr) return false;
    if (type->value() != "entity")  return false;

    StoredAttr* entity = item->get("entity");
    if (entity->getAsSTR() == nullptr) return false;
    if (entity->value() != "area")  return false;

    string categoryStr ("unknown");
    StoredAttr* category = item->get("category");
    if (category  != nullptr) {
        categoryStr = category->value().toStdString();
    }

    Point2FCol points;

    for (int i = 0; true; i++) {
        StoredAttr* pa = item->get("point", i);
        if (pa == nullptr) break;
        StoredAttrNUM* p0 = pa->getAsNUM();
        if (p0 == nullptr) continue;
        points.add(Point2<double>( p0->get(0), p0->get(1)) );
    }

    if (points.count() < 3) return false;

    Color color = Color::BLACK;
    bool color_use = false;
    StoredAttr* colorAttr = item->get("color");
    if ( colorAttr != nullptr ) {
        StoredAttrNUM* colorAttrNUM = colorAttr->getAsNUM();
        if ( colorAttrNUM != nullptr ) {
            color = Color( colorAttrNUM->get(0), colorAttrNUM->get(1), colorAttrNUM->get(2) );
            color_use = true;
        }
    }

    Color colorLine = Color::BLACK;
    bool color_line_use = false;
    StoredAttr* colorLineAttr = item->get("color_line");
    if ( colorLineAttr != nullptr ) {
        StoredAttrNUM* colorLineAttrNUM = colorLineAttr->getAsNUM();
        if ( colorLineAttrNUM != nullptr ) {
            colorLine = Color( colorLineAttrNUM->get(0), colorLineAttrNUM->get(1), colorLineAttrNUM->get(2) );
            color_line_use = true;
        }
    }

    new CadObject_reference( points, color, colorLine, file, color_use, color_line_use );

    return true;
}

//=========================================================================
void CadObject_reference::saveToStored(StoredItem& item, GFile* /*file*/)
{
    item.add(new StoredAttrSTR("type",     "entity"));
    item.add(new StoredAttrSTR("entity",   "area"));

    for (int i = 0; i < points().count(); i++) {
        StoredAttrNUM* attrPt = new StoredAttrNUM("point");
        attrPt->add(points().get(i).x());
        attrPt->add(points().get(i).y());
        item.add(attrPt);
    }

    if (color_use_) {
        StoredAttrNUM* attrColor = new StoredAttrNUM("color");
        attrColor->add(color_.r());
        attrColor->add(color_.g());
        attrColor->add(color_.b());
        item.add(attrColor);
    }

    if (color_line_use_) {
        StoredAttrNUM* attrColorLine = new StoredAttrNUM("color_line");
        attrColorLine->add(colorLine_.r());
        attrColorLine->add(colorLine_.g());
        attrColorLine->add(colorLine_.b());
        item.add(attrColorLine);
    }
}

//=========================================================================
string CadObject_reference::print()
{
    stringstream ss;

    for (int i = 0; i < points_.count(); i++) {
        ss << "xy: ";
        Point2F pti = points_.get(i);
        ss << pti.x() << " " << pti.y();
    }

    return ss.str();
}

//=========================================================================
