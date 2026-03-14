//
// Copyright (C) 2021 Petr Talla. [petr.talla@gmail.com]
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
//self
#include "T2lCmds_cadPro.h"

#include "TcArgCol.h"
#include "T2lCmdQueue.h"

#include "T2lCadSettings.h"
#include "T2lCmd_draw_linePro.h"
#include "T2lCmd_draw_arc.h"
#include "T2lCmd_draw_arc2.h"
#include <QInputDialog>

#include "T2lActiveFile.h"
#include <QTextStream>
#include <T2lGFile.h>
#include "T2lStoredFileNames.h"

#include "T2lGFileCol.h"
#include "T2lCadObject_linePro.h"

#include "TcCmdTransl.h"
#include "TcArgVal.h"

#include "T2lCadAttr_settings.h"
#include "T2lCmd_object_split.h"
#include "T2lCmd_object_movePoints.h"

#include <QDesktopServices>
#include <QFile>

#include <iostream>

using namespace std;
using namespace T2l;


//=============================================================================
int Cmds_cadPro::cmd_object_movepoints(TcCmdContext* /*context*/, TcArgCol& /*args*/)
{
    CmdQueue::queue().add( new Cmd_object_movePoints(), false );
    return 0;
}

//=============================================================================
int Cmds_cadPro::cmd_cad_draw_line_pro(TcCmdContext* /*context*/, TcArgCol& args)
{
    CmdQueue::queue().add( new Cmd_draw_linePro(), false );
    return 0;
}

//=============================================================================
int Cmds_cadPro::cmd_cad_draw_arc(TcCmdContext* /*context*/, TcArgCol& args)
{
    CmdQueue::queue().add( new Cmd_draw_arc(), false );
    return 0;
}

//=============================================================================
int Cmds_cadPro::cmd_cad_draw_arc2(TcCmdContext* /*context*/, TcArgCol& args)
{
    CmdQueue::queue().add( new Cmd_draw_arc2(), false );
    return 0;
}

//=============================================================================
int Cmds_cadPro::cmd_cad_set_line_style(TcCmdContext* /*context*/, TcArgCol& args)
{
    if ( args.count() <= 1 ) return args.appendError("you must enter the symbol");
    TcArg* arg1 = args.at(1);

    string text;
    arg1->toString(text);

    ATTR_SETTINGS_STYLE.set(text.c_str());

    return 0;
}

//=============================================================================
int Cmds_cadPro::cmd_object_split(TcCmdContext* /*context*/, TcArgCol& /*args*/)
{
    CmdQueue::queue().add( new Cmd_object_split(), false );
    return 0;
}

QString createDxfLine(CadObject_linePro* line)
{
    Point2F p0 = line->points().get(0);
    Point2F p1 = line->points().get(1);

    QString style = "CONTINUOUS";
    if(line->style()      == "dashed")     style = "DASHED2";
    else if(line->style() == "dotted")     style = "DOT2";
    else if(line->style() == "dashdot")    style = "DASHDO2";
    else if(line->style() == "dashdotdot") style = "DIVIDE2";

    QString result =
"LINE\n"
"  5\n"
"3A\n"
"100\n"
"AcDbEntity\n"
"  8\n"
"$$$LAYER$$$\n"
"  6\n"
"$$$STYLE$$$\n"
" 62\n"
"    7\n"
"420\n"
"$$$COLOR$$$\n"
"370\n"
"   $$$WIDTH$$$\n"
"100\n"
"AcDbLine\n"
" 10\n"
"$$$X0$$$\n"
" 20\n"
"$$$Y0$$$\n"
" 11\n"
"$$$X1$$$\n"
" 21\n"
"$$$Y1$$$\n"
"  0\n";

    result.replace("$$$X0$$$", QString::number(p0.x()));
    result.replace("$$$Y0$$$", QString::number(p0.y()));
    result.replace("$$$X1$$$", QString::number(p1.x()));
    result.replace("$$$Y1$$$", QString::number(p1.y()));
    result.replace("$$$STYLE$$$", style);

    Color c = line->color();

    unsigned long r = c.r();
    unsigned long g = c.g();
    unsigned long b = c.b();

    QString sufix = "";
    double width = 0;
    QString layer = "nove";
    if(r==0 && g==0 && b==255) {
        layer = "puvodni";
    }
    if(r ==255 && g==165 && b==0) {
        layer = "bourani";
        cout << "========================" << endl;
    }
    if (b==0){
        cout << "hallo" << endl;
    }

    cout << r << " " << g << " " << b << endl;

    if ( line->width()>0.3 ) {
        sufix = "_tluste";
        //width = 3000;
    }
    layer += sufix;
    //cout << width << " " << line->width() << endl;
    //if ( width == 200 )    width = 2000000;
    //if ( width == 100 )    width = 1000000;
    //if ( width == 50 )     width = 500000;
    //if ( width == 25 )     width = 25000;

    if (r==g && g==b) {
        result.replace("420\n$$$COLOR$$$\n", "");
    }
    else {
        r = (r << 16);
        g = (g << 8);
        unsigned long cc = r+g+b;

        //cout << cc << endl;
        result.replace("$$$COLOR$$$", QString::number(cc));
    }



    result.replace("$$$WIDTH$$$", QString::number(0));
    result.replace("$$$LAYER$$$", layer);

    return result;
}

//=============================================================================
int Cmds_cadPro::cmd_cad_export_dxf(TcCmdContext* /*context*/, TcArgCol& args)
{
    QString fileNameSeed = StoredFileNames::getExeUpDir() + "/resource/seed.dxf";
    QFile fileSeed(fileNameSeed);
    fileSeed.open(QIODevice::ReadOnly | QFile::Text);
    QTextStream in(&fileSeed);
    QString dxf = in.readAll();
    fileSeed.close();

    //----------------------------------
    QString filePath = ActiveFile::activeGet()->file()->filePath();
    filePath.replace("t2d", "dxf");

    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream stream(&file);

    QString entities;
    QTextStream entitiesStream(&entities);

    for (int i = 0; i < GFileCol::instance().count(); i++) {
        GFile* file = GFileCol::instance().get(i);
        if (file->loaded() == false) continue;
        for (int i = 0; i < file->objects().count(); i++) {
            CadObject_linePro* line = dynamic_cast<CadObject_linePro*>(file->objects().get(i));
            if (line == nullptr) continue;

            entitiesStream << createDxfLine(line);

        }
    }

    entitiesStream.flush();

    dxf = dxf.replace("$$$ENTITIES$$$", entities);
    stream << dxf;

    stream.flush();
    file.close();

    return 0;
}

//=============================================================================
int Cmds_cadPro::cmd_cad_import_dxf(TcCmdContext* /*context*/, TcArgCol& args)
{
    QFile file("C:/HOME/TIMEAXIS/2022/01/08DumvykresyIn/section.dxf");
    file.open(QIODevice::ReadOnly | QFile::Text);
    QTextStream in(&file);
    QStringList lines;
    while( !in.atEnd()) {
        lines.append(in.readLine());
    }

    QFile f("C:/HOME/TIMEAXIS/2022/01/08DumvykresyIn/section.t2d");
    f.open(QIODevice::WriteOnly | QFile::Text);
    QTextStream out(&f);

    for (int i = 0; i < lines.count()-50; i++ ) {
        if (lines[i] != "AcDbLine") continue;
        if (lines[i+1] != " 10") continue;
        if (lines[i+3] != " 20") continue;
        if (lines[i+7] != " 11") continue;
        if (lines[i+9] != " 21") continue;

        out << "type:             entity" << "\n";
        out << "entity:           line" << "\n";
        out << "point-num:        -" << lines[i+4] << " " << lines[i+2] << "\n";
        out << "point-num:        -" << lines[i+10] << " " << lines[i+8] << "\n";
        out << "color-num:        64 64 64" << "\n";
        out << "width-num:        0.25" << "\n\n";
    }
    out.flush();
    f.close();
}

//=============================================================================
int Cmds_cadPro::cmd_cad_export_svg(TcCmdContext* /*context*/, TcArgCol& args)
{
    //----------------------------------
    QString contentStr;
    QTextStream content(&contentStr);

    for (int i = 0; i < GFileCol::instance().count(); i++) {
        GFile* file = GFileCol::instance().get(i);
        if (file->loaded() == false) continue;
        for (int i = 0; i < file->objects().count(); i++) {
            CadObject_linePro* line = dynamic_cast<CadObject_linePro*>(file->objects().get(i));
            if (line == nullptr) continue;

            content << "<line stroke-width='10' stroke='black' ";
            content << "x1='" << line->points().get(0).x() << "' ";
            content << "y1='" << -line->points().get(0).y() << "' ";
            content << "x2='" << line->points().get(1).x() << "' ";
            content << "y2='" << -line->points().get(1).y() << "'/>\n";
        }
    }

    QString filePath = ActiveFile::activeGet()->file()->filePath();
    filePath.replace("t2d", "svg");

    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream stream(&file);

    stream << "<svg version='1.1'\n"\
              "viewBox='-5000 -20000 25000 25000' width='25000' height='25000'\n"\
              "xmlns='http://www.w3.org/2000/svg'>\n";

    stream << contentStr;

    stream << "</svg>";

    stream.flush();
    file.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));

    return 0;
}


//=============================================================================

