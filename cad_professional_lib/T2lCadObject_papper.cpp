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
#include "T2lCadObject_papper.h"

#include "T2lGFile.h"
#include "T2lCadLine.h"
#include <T2lStyle.h>
#include "T2lEntityList.h"
#include "T2lSfeatArea.h"

#include "T2lStoredItem.h"
#include "T2lEntityLine.h"
#include "T2lEntityArea.h"
#include "T2lEntityText.h"
#include "T2lStoredAttrNUM.h"
#include "T2lStoredAttrSTR.h"
#include "T2lCadObject_symbol.h"

#include "T2lCadSettings.h"
#include "T2lCanvas.h"

#include <QString>

#include <iostream>
#include <sstream>

using namespace T2l;
using namespace std;

//===================================================================
CadObject_papper::CadObject_papper( const Point2F position, double factor, double drawingUnitInMm,
                                    double papperWidthInMm, double papperHeightInMm, GFile* parent,
                                    bool maskLess) :
    ObjectDisplable(Point2FCol(position), parent),
    factor_(factor),
    drawingUnitInMm_(drawingUnitInMm),
    papperWidthInMm_(papperWidthInMm),
    papperHeightInMm_(papperHeightInMm),
    foldWidthInMm_(210),
    maskLess_(maskLess)
{
    if (parent != nullptr) parent->add(this);
}

//===================================================================
CadObject_papper::~CadObject_papper(void)
{
}

//===================================================================
void CadObject_papper::display_line_(EntityList& list, double width,
        const Point2F& p0, const Point2F& p1, const Color& color,
        unsigned char transp, double ext)
{
    Style* style = new Style("");
    style->sfeats().add( new SfeatArea(color, transp));

    Point2F P0 = p0;
    Point2F P1 = p1;

    if ( ext != 0 ) {
        Vector2F e(p0, p1);
        e.setLength(ext);
        P1.add(e);
        e.multiply(-1.0);
        P0.add(e);
    }

    Vector2F d(p0, p1);
    d.setPerpendLeft();
    d.setLength(width);

    Point2F P2 = P1;
    P2.add(d);

    Point2F P3 = P0;
    P3.add(d);

    EntityArea* area = new EntityArea( *style, true, nullptr );
    area->points().points().add(P0);
    area->points().points().add(P1);
    area->points().points().add(P2);
    area->points().points().add(P3);
    list.add( area );
}

//===================================================================
Box2F CadObject_papper::papperBox()
{
    double w = 210*factor_;
    double h = 297*factor_;

    Point2F pt = points().get(0);

    Box2F result;
    result.inflateTo(pt);

    pt.add(Vector2F(w, -h));
    result.inflateTo(pt);

    return result;
}

//===================================================================
void CadObject_papper::display(EntityList& list, RefCol* /*scene*/)
{
    double w = papperWidthInMm_*factor_/drawingUnitInMm_;
    double h = papperHeightInMm_*factor_/drawingUnitInMm_;

    double cm = 10.0*w/papperWidthInMm_;

    double h65 = 6.5;
    if (maskLess_) h65=1;

    //double width  = 0.25;

    Point2F corn = points().get(0);
    Point2F p1   = Point2F(corn.x()+w, corn.y());
    Point2F p2   = Point2F(corn.x()+w, corn.y()-h);
    Point2F p3   = Point2F(corn.x(),   corn.y()-h);

    Point2F pt = p3;
    pt.add(Vector2F(-cm*2, -cm*2));

    //Color cc(255, 253, 250);
    Color cc(255, 255, 255);

    display_line_(list, cm/2,  corn, p3,   cc, 255);
    display_line_(list, h65*cm, p3,   p2,   cc, 255, -cm/2);
    display_line_(list, cm/2,  p2,   p1,   cc, 255);
    display_line_(list, cm/2,  p1,   corn, cc, 255);

    display_line_(list, cm*2.0, corn, p1,   Color::BLACK, 25);
    display_line_(list, cm*2.0, p1,   p2,   Color::BLACK, 25, cm*2.0);
    display_line_(list, cm*2.0, p2,   p3,   Color::BLACK, 25);
    display_line_(list, cm*2.0, p3,   corn, Color::BLACK, 25, cm*2.0);

    corn.add(Vector2F(cm/2, -cm/2));
    p3.add(Vector2F(cm/2, h65*cm));
    p2.add(Vector2F(-cm/2, h65*cm));
    p1.add(Vector2F(-cm/2, -cm/2));
    list.add(new EntityLine(corn, p1, Color::GRAY_LIGHT, 0.18));
    list.add(new EntityLine(p1, p2, Color::GRAY_LIGHT, 0.18));
    list.add(new EntityLine(p2, p3, Color::GRAY_LIGHT, 0.18));
    list.add(new EntityLine(p3, corn, Color::GRAY_LIGHT, 0.18));

    stringstream ss;
    ss << "factor: " << factor_
       << " drawing units [mm]: " << drawingUnitInMm_
       << " papper width [mm]: "  << papperWidthInMm_
       << " papper height [mm]: " << papperHeightInMm_;
    EntityText* textEnt = new EntityText( ss.str().c_str(), pt,
                            POSITION_H_LEFT, POSITION_V_TOP,
                            Style::createTextStyle(Color::GRAY, 2, "", false), true);
    list.add(textEnt);

    displayChange_(list);
}

//=========================================================================
bool CadObject_papper::loadFromStored( StoredItem* item, GFile* file )
{
    StoredAttr* type = item->get("type");
    if (type->getAsSTR() == nullptr) return false;
    if (type->value() != "entity")  return false;

    StoredAttr* entity = item->get("entity");
    if (entity->getAsSTR() == nullptr) return false;
    if (entity->value() != "papper")  return false;

    StoredAttr* pa = item->get("point");
    StoredAttrNUM* p0 = pa->getAsNUM();
    Point2F point( p0->get(0), p0->get(1));

    double factor = CAD_SETTINGS.papperFactor();
    StoredAttr* factorAttr = item->get("scale_factor");
    if ( factorAttr != nullptr ) {
        StoredAttrNUM* factorAttrNUM = factorAttr->getAsNUM();
        if ( factorAttrNUM != nullptr ) {
            factor = factorAttrNUM->get(0);
        }
    }

    double drawing_unit_in_mm = CAD_SETTINGS.drawingUnitInMm();
    StoredAttr* unitAttr = item->get("drawing_unit_in_mm");
    if ( unitAttr != nullptr ) {
        StoredAttrNUM* unitAttrNUM = unitAttr->getAsNUM();
        if ( unitAttrNUM != nullptr ) {
            drawing_unit_in_mm = unitAttrNUM->get(0);
        }
    }

    double papper_width_in_mm = CAD_SETTINGS.papperWidthInMm();
    StoredAttr* widthAttr = item->get("papper_width_in_mm");
    if ( widthAttr != nullptr ) {
        StoredAttrNUM* widthAttrNUM = widthAttr->getAsNUM();
        if ( widthAttrNUM != nullptr ) {
             papper_width_in_mm = widthAttrNUM->get(0);
        }
    }

    double papper_height_in_mm = CAD_SETTINGS.papperHeightInMm();
    StoredAttr* heightAttr = item->get("papper_height_in_mm");
    if ( heightAttr != nullptr ) {
        StoredAttrNUM* heightAttrNUM = heightAttr->getAsNUM();
        if ( heightAttrNUM != nullptr ) {
             papper_height_in_mm = heightAttrNUM->get(0);
        }
    }

    bool papper_maskless = false;
    StoredAttr* maskAttr = item->get("papper_maskless");
    if ( maskAttr != nullptr ) {
        StoredAttrNUM* heightAttrNUM = maskAttr->getAsNUM();
        if ( heightAttrNUM != 0) {
             papper_maskless = true;
        }
    }

    new CadObject_papper( point, factor, drawing_unit_in_mm,
                          papper_width_in_mm, papper_height_in_mm, file, papper_maskless );

    return true;
}

//===================================================================
CadObject_symbol::EIdentified CadObject_papper::identifiedByPoint(const Canvas& canvas, const Point2F& pt)
{
    if (parent_ == nullptr) return IDENTIFIED_NO;

    Point2F ptx(2, 2);
    ptx = canvas.mapPaperToReal(ptx);
    double exp = ptx.x()/1000.0;

    Box2F box;
    box.inflateTo( points().get(0) );
    box.inflateBy( exp );
    if ( box.isInside(pt) ) return IDENTIFIED_YES;
    return IDENTIFIED_NO;
}

//=========================================================================
void CadObject_papper::saveToStored(StoredItem& item, GFile* /*file*/)
{
    item.add(new StoredAttrSTR("type",     "entity"));
    item.add(new StoredAttrSTR("entity",   "papper"));

    StoredAttrNUM* attrPt = new StoredAttrNUM("point");
    attrPt->add(points().get(0).x());
    attrPt->add(points().get(0).y());
    item.add(attrPt);

    StoredAttrNUM* attrScaleFactor = new StoredAttrNUM("scale_factor", factor_);
    item.add(attrScaleFactor);

    StoredAttrNUM* attrUnit = new StoredAttrNUM("drawing_unit_in_mm", drawingUnitInMm_);
    item.add(attrUnit);

    StoredAttrNUM* attrWidth = new StoredAttrNUM("papper_width_in_mm", papperWidthInMm_);
    item.add(attrWidth);

    StoredAttrNUM* attrHeight = new StoredAttrNUM("papper_height_in_mm", papperHeightInMm_);
    item.add(attrHeight);

    StoredAttrNUM* attrMarkless = new StoredAttrNUM("papper_maskless", maskLess_?1:0);
    item.add(attrMarkless);
}

//=========================================================================
string CadObject_papper::print()
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
