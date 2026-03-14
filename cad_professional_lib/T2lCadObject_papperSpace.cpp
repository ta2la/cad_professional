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
#include "T2lCadObject_papperSpace.h"

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
CadObject_papperSpace::CadObject_papperSpace( const Point2F position, double scale, double drawingUnitInMm,
                                    double papperWidthInMm, double papperHeightInMm, GFile* parent ) :
    ObjectDisplable(Point2FCol(position), parent),
    scale_(scale),
    drawingUnitInMm_(drawingUnitInMm),
    papperWidthInMm_(papperWidthInMm),
    papperHeightInMm_(papperHeightInMm)
{
    if (parent != nullptr) parent->add(this);
}

//===================================================================
CadObject_papperSpace::~CadObject_papperSpace(void)
{
}

//===================================================================
void CadObject_papperSpace::display_text(EntityList& list, const string& text, double offset)
{
    double h = papperHeightInMm_*scale_/drawingUnitInMm_;
    Point2F pt(points().get(0).x(), points().get(0).y()-h);

    list.add( new EntityText( text.c_str(), pt,
        POSITION_H_LEFT, POSITION_V_TOP,
        Style::createTextStyle(Color::GRAY, 2, "", false), true,
        AngleXcc(0), Point2F(0, offset)));
}

//===================================================================
void CadObject_papperSpace::display(EntityList& list, RefCol* /*scene*/)
{
    double w = papperWidthInMm_*scale_/drawingUnitInMm_;
    double h = papperHeightInMm_*scale_/drawingUnitInMm_;

    Point2F corn = points().get(0);
    Point2F p1   = Point2F(corn.x()+w, corn.y());
    Point2F p2   = Point2F(corn.x()+w, corn.y()-h);
    Point2F p3   = Point2F(corn.x(),   corn.y()-h);

    list.add( new  EntityLine( corn, p1, Color::GRAY, 0.3 ) );
    list.add( new  EntityLine( p1, p2, Color::GRAY, 0.3 ) );
    list.add( new  EntityLine( p2, p3, Color::GRAY, 0.3 ) );
    list.add( new  EntityLine( p3, corn, Color::GRAY, 0.3 ) );

    stringstream ss;
    ss << "scale: " << scale_;
    display_text(list, ss.str(), -2);

    ss.str("");
    ss << "drawing units [mm]: " << drawingUnitInMm_;
    display_text(list, ss.str(), -3.5);

    ss.str("");
    ss << "papper width [mm]: "  << papperWidthInMm_;
    display_text(list, ss.str(), -5);

    ss.str("");
    ss << "papper height [mm]: " << papperHeightInMm_;
    display_text(list, ss.str(), -6.5);

    ss.str("");
    ss << "targets: ";

    double offset = -8;
    for (string target: targets_) {
        ss << target;
        display_text(list, ss.str(), offset);
        ss.str("");
        offset += 1.5;
    }

    displayChange_(list);
}

//=========================================================================
bool CadObject_papperSpace::loadFromStored( StoredItem* item, GFile* file )
{
    StoredAttr* type = item->get("type");
    if (type->getAsSTR() == nullptr) return false;
    if (type->value() != "entity")  return false;

    StoredAttr* entity = item->get("entity");
    if (entity->getAsSTR() == nullptr) return false;
    if (entity->value() != "papper_space")  return false;

    StoredAttr* pa = item->get("point");
    StoredAttrNUM* p0 = pa->getAsNUM();
    Point2F point( p0->get(0), p0->get(1));

    double factor = CAD_SETTINGS.papperFactor();
    StoredAttr* factorAttr = item->get("scale");
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

    CadObject_papperSpace* ps =
    new CadObject_papperSpace( point, factor, drawing_unit_in_mm,
                          papper_width_in_mm, papper_height_in_mm, file);

    for (int i = 0; ; i++) {
        StoredAttr* attrTarget = item->get("target", i);
        if (attrTarget == nullptr) break;
        ps->appendTarget(attrTarget->value());
    }

    return true;
}

//===================================================================
CadObject_symbol::EIdentified CadObject_papperSpace::identifiedByPoint(const Canvas& canvas, const Point2F& pt)
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
void CadObject_papperSpace::saveToStored(StoredItem& item, GFile* /*file*/)
{
    item.add(new StoredAttrSTR("type",     "entity"));
    item.add(new StoredAttrSTR("entity",   "papper_space"));

    StoredAttrNUM* attrPt = new StoredAttrNUM("point");
    attrPt->add(points().get(0).x());
    attrPt->add(points().get(0).y());
    item.add(attrPt);

    StoredAttrNUM* attrScaleFactor = new StoredAttrNUM("scale", scale_);
    item.add(attrScaleFactor);

    StoredAttrNUM* attrUnit = new StoredAttrNUM("drawing_unit_in_mm", drawingUnitInMm_);
    item.add(attrUnit);

    StoredAttrNUM* attrWidth = new StoredAttrNUM("papper_width_in_mm", papperWidthInMm_);
    item.add(attrWidth);

    StoredAttrNUM* attrHeight = new StoredAttrNUM("papper_height_in_mm", papperHeightInMm_);
    item.add(attrHeight);

    for (int i = 0; i < targets_.size(); i++) {
        StoredAttrSTR* attrFile = new StoredAttrSTR("target", targets_[i].c_str());
        item.add(attrFile);
    }
}

//=========================================================================
CadObject_papperSpace::Targets CadObject_papperSpace::targetsFromString(const string& target)
{
    Targets result;

    std::istringstream iss(target);
    std::string token;

    while (std::getline(iss, token, '|')) {
        result.push_back(token);
    }

    return result;
}

//=========================================================================
string CadObject_papperSpace::targetsToString(const CadObject_papperSpace::Targets& targets)
{
    string result;

    for (auto it = targets.begin(); it != targets.end(); it++) {
        if (it != targets.begin()) result += "|";
        result += *it;
    }

    return result;
}

//=========================================================================
void CadObject_papperSpace::settingsApply()
{
    scale_            = CAD_SETTINGS.papperFactor();
    drawingUnitInMm_  = CAD_SETTINGS.drawingUnitInMm();
    papperWidthInMm_  = CAD_SETTINGS.papperWidthInMm();
    papperHeightInMm_ = CAD_SETTINGS.papperHeightInMm();

    targets_ = targetsFromString(CAD_SETTINGS.printFiles().toStdString());
}

//=========================================================================
void CadObject_papperSpace::settingsExport()
{
    CAD_SETTINGS.papperFactorSet(scale_);
    CAD_SETTINGS.drawingUnitInMmSet(drawingUnitInMm_);
    CAD_SETTINGS.papperWidthInMmSet(papperWidthInMm_);
    CAD_SETTINGS.papperHeightInMmSet(papperHeightInMm_);

    CAD_SETTINGS.printFilesSet(QString::fromStdString(targetsToString(targets_)));
}

//=========================================================================
