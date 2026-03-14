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
#include "T2lCmd_print.h"
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

#include "T2lCadObject_papper.h"

#include <QTextStream>
#include "T2lFilter.h"

#include "T2lPapperEx.h"

#include <QPixmap>
#include "T2lCanvasPainter.h"
#include <QFileInfo>
#include <QDir>

#include <iostream>
#include <assert.h>
#include <sstream>

using namespace T2l;
using namespace std;

//===================================================================
Cmd_print::Cmd_print(void) :
    Cmd("print")
{
}

//===================================================================
Cmd_print::~Cmd_print(void)
{
}

//===================================================================
void Cmd_print::enterPoint( const Point2F& pt, Display& view )
{
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;

    UpdateLock l;

    //file name
    QFileInfo pf(printFile_());
    QDir dir = pf.dir();
    if(dir.exists()==false) {
        dir.cdUp();
        dir.mkdir("print");
    }

    double w          = CAD_SETTINGS.qprint_img_w();
    double h          = CAD_SETTINGS.qprint_img_h();
    double scale      = CAD_SETTINGS.qprint_scale();
    double ppm        = CAD_SETTINGS.qprint_ppm();
    double pix_to_pap = CAD_SETTINGS.qprint_pix_to_pap();
    double du_in_mm   = CAD_SETTINGS_DWG.drawingUnitInMm();

    //printing
    if (CAD_SETTINGS.qprint_use() == 1) { // USING SINGLE POINT
        PapperEx papper( w*pix_to_pap,
                         h*pix_to_pap,
                         scale/pix_to_pap/du_in_mm,
                         ppm );
        Point2F pt0(pt.x(), pt.y()-h*scale/du_in_mm);
        papper.print2(pt0);
        papper.pixmap().save( pf.absoluteFilePath().toStdString().c_str() );
    }
    else if (CAD_SETTINGS.qprint_use() == 2) { // 2 POINTS
        if (cmdpts_.count() == 0) {
            cmdpts_.add(pt);
        }
        else {
            CadSettings cs = CAD_SETTINGS;

            Box2F boundSq = PapperEx::boundSq(cmdpts_.get(0), pt);

            PapperEx papper( CAD_SETTINGS.qprint_img_w()*CAD_SETTINGS.qprint_pix_to_pap(),
                             CAD_SETTINGS.qprint_img_h()*CAD_SETTINGS.qprint_pix_to_pap(),
                             boundSq.x().getLength()/CAD_SETTINGS.qprint_img_w()/CAD_SETTINGS.qprint_pix_to_pap()/CAD_SETTINGS_DWG.drawingUnitInMm(),
                             CAD_SETTINGS.qprint_ppm() );
            papper.print2( boundSq.getPoint(0));
            papper.pixmap().save( pf.absoluteFilePath().toStdString().c_str() );

            cmdpts_.clean();
        }
    }
    else {
            GFile* file = ActiveFile::activeGet()->file();
            if ( file == nullptr ) return;

            //mining the papper object
            CadObject_papper* papperObject = nullptr;

            for ( int i = 0; i < file->objects().count(); i++ ) {
                  papperObject = dynamic_cast<CadObject_papper*>(file->objects().get(i));
                if ( papperObject != nullptr ) break;
            }

            if ( papperObject == nullptr ) return;

            double w = papperObject->papperWidthInMm();
            double h = papperObject->papperHeightInMm();


            PapperEx papper( w*10, h*10, papperObject->factor()/10, 8000 );
            Point2F pt = papperObject->points().get(0);
            Point2F pt0(pt.x(), pt.y()-h*50/1);
            papper.print2( pt0 );
            papper.pixmap().save( pf.absoluteFilePath().toStdString().c_str() );
    }
    /*    if (CAD_SETTINGS.qprint_use()) {
        CadSettings cs = CAD_SETTINGS;

        PapperEx papper( CAD_SETTINGS.qprint_img_w(),
                         CAD_SETTINGS.qprint_img_h(),
                         CAD_SETTINGS.qprint_scale(),
                         CAD_SETTINGS.qprint_scale2(),
                         CAD_SETTINGS.qprint_factor(),
                         CAD_SETTINGS.qprint_ppm() );
        papper.print( pt );
        papper.pixmap().save( pf.absoluteFilePath().toStdString().c_str() );
    }*/

    pack->cleanDynamic();
    pack->dynamicRefresh();

    return;
}

//===================================================================
QString Cmd_print::printFile_()
{
    GFile* file = ActiveFile::activeGet()->file();
    if ( file == nullptr ) return "";

    QFileInfo fi(file->filePath());
    //QDir dir = fi.dir();
    //QString dirStr = dir.absoluteFilePath("print");
    //if(QDir(dirStr).exists()==false) {
    //    dir.mkdir("print");
    //}
    QString outfile = fi.dir().path() + "/print/" + fi.baseName() + ".png";
    return outfile;
}

//===================================================================
void Cmd_print::enterReset( Display& view )
{
}

//===================================================================
void Cmd_print::enterMove( const Point2F& pt, Display& view )
{   
    EntityPack* pack = view.entityPack();
    if ( pack == nullptr ) { assert(0); return; }
    if ( pack->scene() == nullptr ) return;
	
    //<STEP> Dynamic drawing
    pack->cleanDynamic();

    if ( CAD_SETTINGS.qprint_use() == 1 ) {
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
    }

    pack->dynamicRefresh();
}

//===================================================================
QString Cmd_print::hint(void) const
{
    QString result;
    QTextStream stream(&result);

    if ( cmdpts_.count() == 0) {
        stream << "enter first point or reset";
    }
    else if (cmdpts_.count() == 1) {
        Point2F pt = cmdpts_.get(0);
        stream << "position: " << pt.x() << "," << pt.y();
        stream << " - enter next point or reset";
    }
    else {
        double dist = 0;

        for ( int i = 1; i < cmdpts_.count(); i++ ) {
            Point2F p0 = cmdpts_.get(i-1);
            Point2F p1 = cmdpts_.get(i);

            dist += Vector2F(p0, p1).getLength();
        }

        stream << "length: " << dist;
        stream << " - enter next point or reset";
    }

    stream.flush();
    return result;
}

//===================================================================
QString Cmd_print::dialogTml() const
{
    QString result;

    //result += "TC;control: text;text: CHACK;;";

    result += "TC;control: text;";
    result += "text: ";
    if (CAD_SETTINGS.qprint_use() == 1) {
        result += "using quick print;";
        result += "cmd: cad_set_qprint_use 3;;";
    }
    /*else if (CAD_SETTINGS.qprint_use() == 2) {
        result += "using quick print - 2 points entry;";
        result += "cmd: cad_set_qprint_use 1;;";
    }*/
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
    }

    //===================================================
    result = result.replace("TC", "type: control");
    result = result.replace("CT", "control: text");
    result = result.replace("CB", "control: button");
    result = result.replace("CE", "control: edit");
    result = result.replace(";", "\n");

    return result;
}

//===================================================================
