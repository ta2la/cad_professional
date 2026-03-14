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
#include "T2lCmd_print_papperSpace.h"
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
#include "T2lEntityText.h"
#include "T2lEntityPoint.h"

#include "T2lCadObject_papperSpace.h"
#include "T2lCadObject_text.h"

#include "T2lFilter.h"
#include "T2lPapperEx.h"
#include "T2lCanvasPainter.h"

#include <QTextStream>
#include <QPixmap>
#include <QFileInfo>
#include <QDir>

#include <iostream>
#include <assert.h>
#include <sstream>
#include <regex>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_print_papperSpace::Cmd_print_papperSpace(void) :
    Cmd("print")
{
}

//===================================================================
Cmd_print_papperSpace::~Cmd_print_papperSpace(void)
{
}

//===================================================================
Cmd_print_papperSpace::Substitutions Cmd_print_papperSpace::substitutions(GFile* file)
{
    Cmd_print_papperSpace::Substitutions result;

    CadObject_text* text = nullptr;

    regex re("\\$\\$(.*?)\\$\\$=(.*)");

    for ( int i = 0; i < file->objects().count(); i++ ) {
        text = dynamic_cast<CadObject_text*>(file->objects().get(i));
        if ( text == nullptr ) continue;

        string textStr = text->text().toStdString();

        if (!regex_match(textStr, re)) continue;

        smatch match;
        regex_search(textStr, match, re);
        assert(match.size()>=2);

        string id = match[1];
        string value = match[2];

        result.push_back(std::make_tuple(match[1], match[2]));
    }

    return result;
}

//===================================================================
void Cmd_print_papperSpace::enterPoint( const Point2F& /*pt*/, Display& view )
{
    //<STEP>
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    GFile* file = ActiveFile::activeGet()->file();
    if ( file == nullptr ) return;

    UpdateLock l;

    QDir dir = QFileInfo(file->filePath()).dir(); //base directory for relative paths

    double ppm        = CAD_SETTINGS.qprint_ppm();
    double pix_to_pap = CAD_SETTINGS.qprint_pix_to_pap();

    Substitutions substs = substitutions(file);

    //<STEP> enumerating all papper objects
    CadObject_papperSpace* papperObject = nullptr;

    for ( int i = 0; i < file->objects().count(); i++ ) {
        papperObject = dynamic_cast<CadObject_papperSpace*>(file->objects().get(i));
        if ( papperObject == nullptr ) continue;

        double w = papperObject->papperWidthInMm();
        double h = papperObject->papperHeightInMm();
        double scale      = papperObject->scale();
        double du_in_mm   = papperObject->drawingUnitInMm();
        Point2F pt0 = papperObject->points().get(0);
        pt0 = Point2F(pt0.x(), pt0.y()-h*scale/du_in_mm);

        PapperEx papper( w*pix_to_pap,
                         h*pix_to_pap,
                         scale/pix_to_pap/du_in_mm,
                         ppm );
        papper.print2(pt0);

        for ( auto target : papperObject->targets_ ) {
            string targetStr = target;

            for (auto subst : substs) {
                string& var = get<0>(subst);
                size_t found = targetStr.find(string("$$") + var + "$$");
                if (found == string::npos) continue;
                targetStr.replace(found, var.size()+4, get<1>(subst));
            }

            QString filePath = dir.filePath(QString::fromStdString(targetStr));
            QDir().mkpath(QFileInfo(filePath).path());
            papper.pixmap().save( filePath.toStdString().c_str() );
        }
    }

    //<STEP>
    pack->cleanDynamic();
    pack->dynamicRefresh();

    return;
}

//===================================================================
void Cmd_print_papperSpace::enterReset( Display& /*view*/ )
{
}

//===================================================================
void Cmd_print_papperSpace::enterMove( const Point2F& pt, Display& view )
{   
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;
	
    //<STEP> Dynamic drawing
    pack->cleanDynamic();

    /*if ( CAD_SETTINGS.qprint_use() == 1 ) {
        Box2F bound;
        bound.inflateTo(pt);
        Point2F pt2 = pt;
        pt2.add( Vector2F( CAD_SETTINGS.qprint_img_w()*CAD_SETTINGS.qprint_scale()/CAD_SETTINGS.drawingUnitInMm(),
                           -CAD_SETTINGS.qprint_img_h()*CAD_SETTINGS.qprint_scale()/CAD_SETTINGS.drawingUnitInMm() ) );
        bound.inflateTo(pt2);
        EntityLine* line = new EntityLine( Color::GRAY_DARK, 0.5 );
        for (int i = 0; i < 5; i++) {
            line->points().points().add(bound.getPoint(i));
        }
        pack->addDynamic(line);
    }
    else if ( CAD_SETTINGS.qprint_use()==2 && cmdpts_.count()>=1 ) {
        Box2F boundSq = PapperEx::boundSq(cmdpts_.get(0), pt);

        EntityLine* line = new EntityLine( Color::GRAY_DARK, 0.5 );
        for (int i = 0; i < 5; i++) {
            line->points().points().add(boundSq.getPoint(i));
        }
        pack->addDynamic(line);

        line = new EntityLine( Color::GRAY_DARK, 0.5 );
        line->points().points().add(boundSq.getPoint(0));
        line->points().points().add(boundSq.getPoint(2));
        pack->addDynamic(line);

        Style* styleCircle0 = Style::createPointStyle(Color::RED, Style::SYMBOL_CIRCLE_FILLED, 1, "void");
        pack->addDynamic( new EntityPoint( boundSq.getPoint(2), *styleCircle0, true, ANGLE_ZERO_VIEW, AngleXcc(0), nullptr ) );
    }*/

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_print_papperSpace::hint(void) const
{
    QString result;
    QTextStream stream(&result);

    return "enter point to perform action";
}

//===================================================================
QString Cmd_print_papperSpace::dialogTml() const
{
    QString result;

/*    result += "TC;control: text;";
    result += "text: ";
    if (CAD_SETTINGS.qprint_use() == 1) {
        result += "using quick print;";
        result += "cmd: cad_set_qprint_use 3;;";
    }
    else {
        result += "using papper object;";
        result += "cmd: cad_set_qprint_use 1;;";
    }
    result += "TC;CT;text: <hup>;;";
    result += "TC;CT;text: <hup>;;";

    if (CAD_SETTINGS.qprint_use() != 0) {
        if (CAD_SETTINGS.qprint_use() == 1) {
        result += "TC;CT;text: image size:;;";
        result += "TC;CT;<space>;;";
        result += "TC;control: spacer;spacer: minx;value: 80;;";
        result += "TC;CE;text: " + QString::number(CAD_SETTINGS.qprint_img_w()) + ";";
        result += "cmd: cad_set_qprint_img_w \"$TEXT\";";
        result += "width: 40;;";
        result += "TC;control: edit;";
        result += "text: " + QString::number(CAD_SETTINGS.qprint_img_h()) + ";";
        result += "cmd: cad_set_qprint_img_h \"$TEXT\";";
        result += "width: 40;;";
        result += "TC;CT;text: <hup>;;";

        //===============================
        result += "TC;CT;text: scale:;;";
        result += "TC;CT;<space>;;";
        result += "TC;control: spacer;spacer: minx;value: 80;;";
        result += "TC;CE;text: " + QString::number(CAD_SETTINGS.qprint_scale()) + ";";
        result += "cmd: cad_set_qprint_scale \"$TEXT\";";
        result += "width: 40;;";
        result += "TC;CT;text: <hup>;;";

        //================================
        result += "TC;CT;text: ppm: ;;";
        result += "TC;CT;<space>;;";
        result += "TC;control: spacer;spacer: minx;value: 80;;";
        result += "TC;CE;text:" + QString::number(CAD_SETTINGS.qprint_ppm()) + ";";
        result += "cmd: cad_set_qprint_ppm \"$TEXT\";";
        result += "width: 40;;";
        result += "TC;CT;text: <hup>;;";

        //================================
        result += "TC;control: text;text: pixels to mm: ;;";
        result += "TC;CT;<space>;;";
        result += "TC;control: spacer;spacer: minx;value: 80;;";
        result += "TC;CE;text: " + QString::number(CAD_SETTINGS.qprint_pix_to_pap()) + ";";
        result += "cmd: cad_set_qprint_pix_to_pap \"$TEXT\";";
        result += "width: 40;;";
        result += "TC;CT;text: <hup>;;";

        //================================
        result += "TC;control: text;text: drawing unit in [mm]:;;";
        result += "TC;CT;<space>;;";
        result += "TC;control: spacer;spacer: minx;value: 80;;";
        result += "TC;control: edit;";
        result += "TC;CE;text: " + QString::number(CAD_SETTINGS.drawingUnitInMm()) + ";";
        result += "cmd: cad_set_drawing_unit_in_mm \"$TEXT\";";
        result += "width: 40;;";
        }
    }*/

    //===================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace("CE", "control: edit");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
