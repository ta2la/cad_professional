//
// Copyright (C) 2015, 2023 Petr Talla. [petr.talla@gmail.com]
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
#include "T2lCadObject_arc.h"
#include "T2lGFile.h"
#include <T2lStyle.h>
#include "T2lEntityList.h"
#include "T2lCanvas.h"
#include "T2lEntityPoint.h"
#include "T2lObPointXy.h"
#include "T2lSitemLine.h"
#include "T2lSitemArea.h"
#include "T2lLstyle.h"
#include "T2lSfeatSymbol.h"

#include "T2lStoredItem.h"
#include "T2lEntityLine.h"
#include "T2lStoredAttrNUM.h"
#include "T2lStoredAttrSTR.h"
#include "T2lEntityLine.h"
#include <iostream>
#include <sstream>
#include <QString>

#include "T2lStyleChange.h"

using namespace T2l;
using namespace std;

//=============================================================================
CadObject_arc::CadObject_arc( const Point2Col<double>& points, GFile* parent, int gid, const char* style ) :
    //ObjectDisplable(points, parent, gid),
    ObjectDisplable(),
    color_(Color::BLUE),
    width_(0.25),
    style_(style)
{
    if (parent != nullptr) parent->add(this);
    assert(points.count()==3);

    Point2F p0 = points.get(0);
    Point2F p1 = points.get(1);
    Point2F p2 = points.get(2);
    normalize(p0, p1, p2);
    points_.append(ObPointXy(p0));
    points_.append(ObPointXy(p1));
    points_.append(ObPointXy(p2));
}

//=============================================================================
CadObject_arc::~CadObject_arc(void)
{
}

//=============================================================================
void CadObject_arc::display(EntityList& list, RefCol* /*scene*/)
{
    if (parent_ == nullptr) return;

    // <STEP>
/*    EntityLine* line = new EntityLine( *Style::createLineStyleStr(color(), width(), style_.c_str()), true );
    line->points().points().add( points_.get(0));
    line->points().points().add( points_.get(1));
    list.add( line );

    line = new EntityLine( *Style::createLineStyleStr(Color::BLUE, width(), style_.c_str()), true );
    line->points().points().add( points_.get(1));
    line->points().points().add( points_.get(2));
    list.add( line );*/

    EntityLine* circle = new EntityLine( Color(12,57,89), 0.1);
    Point2F p0 = points_.get(0);
    Point2F p1 = points_.get(1);
    Point2F p2 = points_.get(2);
    pointsCurve(circle->points().points(), p0, p1, p2);

    list.add( circle );

    displayChange_(list);
}

//=========================================================================
void CadObject_arc::pointsCurve( Point2FCol& points, Point2F& p0, Point2F& p1, Point2F& p2 )
{
    if ( p0.x()==p2.x() && p0.y()!=p2.y() ) return;

    Point2F xy;
    double r;

    bool exists = CadObject_arc::centerRadius(p0, p1, p2, xy, r);

    if (!exists) return;

    Vector2F dir(xy, p0);
    double minLengthSq = Vector2F(p1,p0).getLengthSq();

    points.add( p0 );

    bool fullCircle = false;
    if  ( Vector2F::angleBetween(Vector2F(p1, p0), Vector2F(p1, p2)).get() < 1 ) fullCircle = true;

    for (int a = 0; a <=360; a+=3) {
        Point2F p = xy;
        p.add(dir);

        if (a!=0 && !fullCircle && Vector2F(p1, p).getLengthSq()>minLengthSq) {
            points.add( p2 );
            break;
        }

        points.add( p );

        dir.rotateCc(3);
    }
}

//=========================================================================
bool CadObject_arc::loadFromStored( StoredItem* item, GFile* file )
{
    StoredAttr* type = item->get("type");
    if (type->getAsSTR() == nullptr) return false;
    if (type->value() != "entity")  return false;

    StoredAttr* entity = item->get("entity");
    if (entity->getAsSTR() == nullptr) return false;
    if (entity->value() != "line")  return false;

    StoredAttr* pa0 = item->get("point", 0);
    if (pa0 == nullptr) return false;
    StoredAttrNUM* p0 = pa0->getAsNUM();
    if (p0 == nullptr) return false;

    StoredAttr* pa1 = item->get("point", 1);
    if (pa1 == nullptr) return false;
    StoredAttrNUM* p1 = pa1->getAsNUM();
    if (p1 == nullptr) return false;

    Point2Col<double> points;
    points.add(Point2<double>( p0->get(0), p0->get(1)) );
    points.add(Point2<double>( p1->get(0), p1->get(1)) );

    Color color = Color::BLACK;
    StoredAttr* colorAttr = item->get("color");
    if ( colorAttr != nullptr ) {
        StoredAttrNUM* colorAttrNUM = colorAttr->getAsNUM();
        if ( colorAttrNUM != nullptr ) {
            color = Color( colorAttrNUM->get(0), colorAttrNUM->get(1), colorAttrNUM->get(2) );
        }
    }

    double width = 0.25;
    StoredAttr* widthAttr = item->get("width");
    if ( widthAttr != nullptr ) {
        StoredAttrNUM* widthAttrNUM = widthAttr->getAsNUM();
        if ( widthAttrNUM != nullptr ) {
            width = widthAttrNUM->get(0);
        }
    }

    int gidValue = 0;
    StoredAttr* gidAttr = item->get("sys_GID");
    if ( gidAttr != nullptr ) {
        if (gidAttr->getAsNUM() != nullptr) {
            gidValue = gidAttr->getAsNUM()->get();
        }
    }

    string style = "solid";
    StoredAttr* style_attr = item->get("style");
    if ( style_attr ) {
        style = style_attr->value().toStdString();
    }

    CadObject_arc* cadLine = new CadObject_arc( points, file, gidValue, style.c_str());
    cadLine->colorSet(color);
    cadLine->widthSet(width);

    StoredAttr* symbol_beg = item->get("symbol_beg");
    if (symbol_beg) {
        cadLine->symbolBeg_ = symbol_beg->value().toStdString();
    }

    StoredAttr* symbol_end = item->get("symbol_end");
    if (symbol_end) {
        cadLine->symbolEnd_ = symbol_end->value().toStdString();
    }
}

//=============================================================================
void CadObject_arc::saveToStored(StoredItem& item, GFile* file)
{
    StoredAttrNUM* attrGID = new StoredAttrNUM("sys_GID");
    attrGID->add(gid());
    item.add(attrGID);

    item.add(new StoredAttrSTR("type",   "entity"));
    item.add(new StoredAttrSTR("entity", "line"));

    StoredAttrNUM* attrBeg = new StoredAttrNUM("point");
    attrBeg->add(points_.get(0).x());
    attrBeg->add(points_.get(0).y());
    item.add(attrBeg);

    StoredAttrNUM* attrEnd = new StoredAttrNUM("point");
    attrEnd->add(points_.get(1).x());
    attrEnd->add(points_.get(1).y());
    item.add(attrEnd);

    StoredAttrNUM* attrColor = new StoredAttrNUM("color");
    attrColor->add(color().r());
    attrColor->add(color().g());
    attrColor->add(color().b());
    item.add(attrColor);

    StoredAttrNUM* attrWidth = new StoredAttrNUM("width");
    attrWidth->add(width());
    item.add(attrWidth);

    if (style() != "solid") {
        item.add(new StoredAttrSTR("style", style().c_str()));
    }

    if (symbolBeg_.empty() == false) {
        item.add(new StoredAttrSTR("symbol_beg", symbolBeg_.c_str()));
    }

    if (symbolEnd_.empty() == false) {
        item.add(new StoredAttrSTR("symbol_end", symbolEnd_.c_str()));
    }
}

//=============================================================================
string CadObject_arc::print()
{
    stringstream ss;
    ss << "LINE: " << "color: " << (int)color().r() << "," << (int)color().g() << "," << (int)color().b();

    return ss.str();
}

//=============================================================================
ObjectDisplable* CadObject_arc::clone()
{
    CadObject_arc* line = new CadObject_arc(Point2FCol(), parent_);
    for ( int i = 0; i < points_.count(); i++ ) line->points_.append(ObPointXy(points_.get(i)));
    line->color_ = color_;
    line->width_ = width_;
    line->style_ = style_;

    return line;
}

//=============================================================================
ObjectDisplable::EIdentified CadObject_arc::identifiedByPoint(const T2l::Canvas& canvas, const Point2F& pt)
{
    EIdentified result = ObjectDisplable::identifiedByPoint(canvas, pt);
    if (result == IDENTIFIED_NO) return result;

    PolyLine2 pline;
    for (int i = 0; i < points().count(); i++) {
        pline.points().add(points().get(i));
    }

    Point2F ptr(0.002, 0.0015);
    ptr = canvas.mapPaperToReal(ptr);
    double d = ptr.x();

    Area2 area2;
    Ray2 ray(points().get(0), points().get(1));
    area2.points().points().add(ray.getPoint(0, d));
    area2.points().points().add(ray.getPoint(0, -d));
    area2.points().points().add(ray.getPoint(1, -d));
    area2.points().points().add(ray.getPoint(1, d));

    if (area2.isInside(pt)) return IDENTIFIED_YES;
    return IDENTIFIED_NO;
}

//=============================================================================
Point2F  CadObject_arc::snapGet(int index)
{
    if (points().count() < 2) return Point2F(10e9, 10e9);

    Point2F p0 = points().get(0);
    Point2F p1 = points().get(1);

    if (index == 0) return p0;
    if (index == 1) return p1;

    if ( index < snapRawCount() ) return snapRawGet(index).xy(); //TODO

    return Point2F( (p0.x()+p1.x())/2.0, (p0.y()+p1.y())/2.0 );
}

//=============================================================================
void CadObject_arc::displayGid_(EntityList& list, RefCol* scene)
{
}

//=============================================================================
void CadObject_arc::normalize(Point2F& p0, Point2F& p1, Point2F& p2)
{
    Point2F PT = p1;
    Vector2F dir(p1, p2);
    dir.setLength(Vector2F(p0, p1).getLength());
    PT.add(dir);

    p2 = PT;

    //if (CadObject_arc::isCcWise(p1, p0, Vector2F(p1, p2))) {
    if (CadObject_arc::isCcWise(p1, p0, Vector2F(p0, p2))) {
        Point2F p = p0;
        p0 = p2;
        p2 = p;
    }

    if ( Vector2F::angleBetween(Vector2F(p1, p0), Vector2F(p1, p2)).get() < 10 ) {
        p2= p0;
    }
}

//=============================================================================
bool CadObject_arc::isCcWise(const Point2F& center, const Point2F& dirPt, const Vector2F& dir)
{
    if ( Vector2F::crossProduct(Vector2F(center, dirPt), dir) > 0) return true;
    return false;

    /*Vector2F checkDir(center, dirPt);
    checkDir.setPerpendLeft();
    double checkSize = checkDir.getLength();
    checkDir.add(dir);
    double checkSize2 = checkDir.getLength();
    if ( checkSize2 > checkSize) return true;
    return false;*/
}

//=============================================================================
bool CadObject_arc::centerRadius(const Point2F& p0, const Point2F& p1, const Point2F& p2, Point2F& center, double& radius)
{
    Vector2F v1(p0, p1);
    Vector2F v2(p1, p2);

    if ( Vector2F::angleBetween(Vector2F(p1, p0), v2).get() < 1 ) {
        //center = Point2F::center( Point2F::center(p1,p0), Point2F::center(p2,p1) );
        center = Point2F::center(p0, p1);
        //center = Point2F( (p0.x()+p1.x())/2.0, (p0.y()+p1.y())/2.0 );
        radius = Vector2F(p1, center).getLength();
        return true;
    }

    v1.setPerpendLeft();
    v2.setPerpendLeft();

    //Point2F c1 = Point2F::center(p0,p1);
    //Point2F c2 = Point2F::center(p1,p2);

    Point2F c1 = Point2F( (p0.x()+p1.x())/2.0, (p0.y()+p1.y())/2.0);
    Point2F c2 = Point2F( (p2.x()+p1.x())/2.0, (p2.y()+p1.y())/2.0);

    Ray2 ray1(c1, v1);
    Ray2 ray2(c2, v2);

    double param;
    bool exists = Ray2::intersectParam(ray1, ray2, param);
    if (!exists) return false;

    center = ray1.getPoint(param);
    radius = Vector2F(center, p2).getLength();

    return true;
}

//=============================================================================
