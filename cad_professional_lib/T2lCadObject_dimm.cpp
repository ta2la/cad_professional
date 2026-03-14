//
// Copyright (C) 2018 Petr Talla. [petr.talla@gmail.com]
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
#include "T2lGFile.h"
#include "T2lCadObject_dimm.h"
#include <T2lStyle.h>
#include "T2lEntityList.h"
#include "T2lEntityText.h"
#include "T2lEntityPoint.h"
#include "T2lObPointXy.h"

#include "T2lStoredItem.h"
#include "T2lEntityLine.h"
#include "T2lStoredAttrNUM.h"
#include "T2lStoredAttrSTR.h"
#include "T2lEntityLine.h"
#include <iostream>
#include <sstream>
#include <QString>

#include "T2lStyleChange.h"

#include "T2lCadSettings.h"

using namespace T2l;
using namespace std;

//===================================================================
CadObject_dimm::CadObject_dimm( const Point2Col<double>& points, double dir, GFile* parent ) :
    ObjectDisplable(points, parent),
    color_(Color::GRAY),
    size_(150),
    dir_(dir)
{
    if (parent != nullptr) parent->add(this);
}

//===================================================================
CadObject_dimm::~CadObject_dimm(void)
{
}

//=============================================================================
ObjectDisplable* CadObject_dimm::clone()
{
    CadObject_dimm* dimm = new CadObject_dimm(Point2FCol(), dir_, parent_);
    dimm->color_ = color_;
    dimm->size_ = size_;
    dimm->metricText_ = metricText_;
    for ( int i = 0; i < points_.count(); i++ ) {
        Point2F pt = points_.get(i);
        dimm->points().append(ObPointXy(pt));
    }
    return dimm;
}

//===================================================================
void CadObject_dimm::calculateBasicGeometry_(Point2F& p0, Point2F& p1)
{
    Ray2 ray(points_.get(0), points_.get(1));
    Point2F ptC = ray.getPoint(ray.nearestParam(points_.get(2)));

    Point2F p0b = points_.get(0);
    p0 = points_.get(2);
    p0.add( Vector2F(ptC, p0b ) );

    Point2F p1b = points_.get(1);
    p1 = points_.get(2);
    p1.add( Vector2F(ptC, p1b ) );

    if (dir_ >= 0) {
        Ray2 ray0(p0b, Vector2F(AngleXcc(dir_), 1));
        p0 = ray0.getPoint(ray0.nearestParam(points_.get(2)));

        Ray2 ray1(p1b, Vector2F(AngleXcc(dir_), 1));
        p1 = ray1.getPoint(ray1.nearestParam(points_.get(2)));
    }
}

//===================================================================
Point2F CadObject_dimm::snapGet(int index)
{
    if (nondisplable()) return Point2F(-1000000,-1000000);

    if (index == 0) return points_.get(0);
    else if (index == 1) return points_.get(1);
    else {
        Point2F p0;
        Point2F p1;

        calculateBasicGeometry_(p0, p1);

        if (index == 2) return p0;
        return p1;
    }
}

//===================================================================
void CadObject_dimm::display(EntityList& list, RefCol* /*scene*/)
{
    if (nondisplable()) return;

    /*Ray2 ray(points_.get(0), points_.get(1));
    Point2F ptC = ray.getPoint(ray.nearestParam(points_.get(2)));

    Point2F p0b = points_.get(0);
    Point2F p0 = points_.get(2); //p0b;
    //p0 = Point2F(p0.x()+Vector2F(p0b, points_.get(1)).getLength(), p0.y());
    p0.add( Vector2F(ptC, p0b ) );

    Point2F p1b = points_.get(1);
    Point2F p1 = points_.get(2); //p1b;
    p1.add( Vector2F(ptC, p1b ) );
    //p1= Point2F(p0.x(), p1.y());

    if (dir_ >= 0) {
        Ray2 ray0(p0b, Vector2F(AngleXcc(dir_), 1));
        p0 = ray0.getPoint(ray0.nearestParam(points_.get(2)));

        Ray2 ray1(p1b, Vector2F(AngleXcc(dir_), 1));
        p1 = ray1.getPoint(ray1.nearestParam(points_.get(2)));
    }*/

    Point2F p0b = points_.get(0);
    Point2F p1b = points_.get(1);

    Point2F p0;
    Point2F p1;

    calculateBasicGeometry_(p0, p1);

    //<STEP> main line
    EntityLine* line = new EntityLine( color_, 0.2 );
    line->points().points().add( p0 );
    line->points().points().add( p1 );
    list.add( line );

    //<STEP> side lines
    Vector2F offset(p0b, p0);
    offset.setLength(1);
    offset.multiply(size_);

    EntityLine* line0 = new EntityLine( color_, 0.1 );
    Point2F p0b_0 = p0b;
    Point2F p0_0 = p0;
    if ( Vector2F(p0b, p0).getLength() > size_) {
        p0b_0.add(offset);
    }
    else {
        p0b_0 = p0;
    }
    p0_0.add(offset);
    line0->points().points().add( p0b_0 );
    line0->points().points().add( p0_0 );
    list.add( line0 );

    EntityLine* line1 = new EntityLine( color_, 0.1 );
    Point2F p1b_0 = p1b;
    Point2F p1_0 = p1;
    if ( Vector2F(p1b, p1).getLength() > size_) {
        p1b_0.add(offset);
    }
    else {
        p1b_0 = p1;
    }
    p1_0.add(offset);
    line1->points().points().add( p1b_0 );
    line1->points().points().add( p1_0 );
    list.add( line1 );

    //<STEP> dimension symbols
    Vector2F dir(p0, p1);
    dir.setLength(1);
    Vector2F markDir = dir;
    markDir.rotateCc(Angle(45));
    markDir.setLength(size_/1.5);

    Point2F markBeg = p0;
    Point2F markEnd = p0;
    markEnd.add(markDir);
    markDir.multiply(-1.0);
    markBeg.add(markDir);

    EntityLine* lineB = new EntityLine( color_, 0.35 );
    lineB->points().points().add( markBeg );
    lineB->points().points().add( markEnd );
    list.add( lineB );

    Point2F mark2Beg = p1;
    Point2F mark2End = p1;
    mark2End.add(markDir);
    markDir.multiply(-1.0);
    mark2Beg.add(markDir);

    EntityLine* line2B = new EntityLine( color_, 0.35 );
    line2B->points().points().add( mark2Beg );
    line2B->points().points().add( mark2End );
    list.add( line2B );

    /*Style* styleCircle0 = Style::createPointStyle(color_, Style::SYMBOL_CIRCLE_FILLED, 1.75, "void");
    list.add( new EntityPoint( p0, *styleCircle0, true, ANGLE_ZERO_VIEW, AngleXcc(0), nullptr ) );

    Style* styleCircle1 = Style::createPointStyle(color_, Style::SYMBOL_CIRCLE_FILLED, 1.75, "void");
    list.add( new EntityPoint( p1, *styleCircle1, true, ANGLE_ZERO_VIEW, AngleXcc(0), nullptr ) );*/

    double x = ( p0.x() + p1.x() ) / 2.0;
    double y = ( p0.y() + p1.y() ) / 2.0;

    Vector2F dist(p0, p1);
    int length = static_cast<int>(dist.getLength());
    length /= 5;
    length *= 5;
    QString lengthStr = QString::number(length);

    Vector2F textOffset(p1, p0);
    textOffset.setLength(0.6*size_*lengthStr.length());
    Point2F textPosition(x,y);
    textPosition.add(textOffset);

    EntityText* text = new EntityText( lengthStr, textPosition, POSITION_H_CENTER, POSITION_V_BOTTOM,
                                Style::createTextStyle(Color::GRAY_DARK, size_, "", false),
                                true, dir.getAngle(), Point2F(0, 0), nullptr, true );

    list.add(text);

    displayChange_(list);
}


//=========================================================================
bool CadObject_dimm::loadFromStored( StoredItem* item, GFile* file )
{
    StoredAttr* type = item->get("type");
    if (type->getAsSTR() == nullptr) return false;
    if (type->value() != "entity")  return false;

    StoredAttr* entity = item->get("entity");
    if (entity->getAsSTR() == nullptr) return false;
    if (entity->value() != "dimm")  return false;

    StoredAttr* pa0 = item->get("point", 0);
    if (pa0 == nullptr) return false;
    StoredAttr* pa1 = item->get("point", 1);
    if (pa1 == nullptr) return false;
    StoredAttr* pa2 = item->get("point", 2);
    if (pa2 == nullptr) return false;

    StoredAttrNUM* p0 = pa0->getAsNUM();
    if (p0 == nullptr) return false;

    StoredAttrNUM* p1 = pa1->getAsNUM();
    if (p1 == nullptr) return false;

    StoredAttrNUM* p2 = pa2->getAsNUM();
    if (p2 == nullptr) return false;

    Point2Col<double> points;
    points.add(Point2<double>( p0->get(0), p0->get(1)) );
    points.add(Point2<double>( p1->get(0), p1->get(1)) );
    points.add(Point2<double>( p2->get(0), p2->get(1)) );

    double dir = -1;
    StoredAttr* dirAttr = item->get("dir");
    if (dirAttr != nullptr) {
        StoredAttrNUM* dirAttrNUM = dirAttr->getAsNUM();
        if (dirAttrNUM != nullptr) {
            dir = dirAttrNUM->get(0);
        }
    }

    CadObject_dimm* dimm = new CadObject_dimm( points, dir, file );

    StoredAttr* sizeAttr = item->get("size");
    if (sizeAttr != nullptr) {
        double size = sizeAttr->value().toDouble();
        dimm->sizeSet(size);
    }

    StoredAttr* metricAttr = item->get("metric");
    if (metricAttr) {
        if (metricAttr->value() == "true") {
            dimm->setMetricText(true);
        }
    }

    return true;
}

//=========================================================================
void CadObject_dimm::saveToStored(StoredItem& item, GFile* /*file*/)
{
    item.add(new StoredAttrSTR("type",   "entity"));
    item.add(new StoredAttrSTR("entity", "dimm"));

    for (int i = 0; i < points_.count(); i++) {
        StoredAttrNUM* attrPt = new StoredAttrNUM("point");
        attrPt->add(points_.get(i).x());
        attrPt->add(points_.get(i).y());
        item.add(attrPt);
    }

    item.add(new StoredAttrSTR("size", QString::number(size())));
    if ( metricText_ ) {
        StoredAttrSTR* attrMetric = new StoredAttrSTR("metric", "true" );
        item.add(attrMetric);
    }

    item.add(new StoredAttrNUM("dir", dir_));
}

//=========================================================================
string CadObject_dimm::print()
{
    stringstream ss;
    ss << "DIMENSION: ";

    return ss.str();
}

//=========================================================================
