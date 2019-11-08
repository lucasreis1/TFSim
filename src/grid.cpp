#include<iostream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/group.hpp>
#include "grid.hpp"
namespace nana
{

grid::grid( window wd, const rectangle& r, int row, int col)
    : listbox( wd, r )
    , myRowCount( 0 )
    , myColCount( 0 )
    , lastPushed( 0 )
{
    Resize(row,col);
    //show_header(false);
}

void grid::Resize( int rows, int cols )
{
    int newRows = rows - myRowCount;
    int newCols = cols - myColCount;
    myRowCount = rows;
    myColCount = cols;

    for( int kcol = 0; kcol < newCols; kcol++ )
    {
        append_header(std::to_string(kcol), 200);
        column_at(kcol).width(40);
    }

    for( int krow = 0; krow < newRows; krow++ )
        at(0).append({"", ""});
}
void grid::ColTitle( int col, const std::string& value )
{
    column_at( col ).text( value );
}
void grid::ColWidth( int col, int width )
{
    column_at( col ).width( width );
}
void grid::Set( int row, int col, const std::string& value )
{
    if( ! CheckIndex( row, col ) )
        return;
    at(listbox::index_pair(0,row)).text(col,value);

}

void grid::Set(int pos, const std::string& value)
{
    int r,c;
    r = pos/myColCount;
    c = pos%myColCount;
    if(!CheckIndex(r,c))
    {
        return;
    }
    Set(r,c,value);
}

void grid::Push(const std::string& value)
{
    int r,c;
    r = lastPushed/myColCount;
    c = lastPushed%myColCount;
    if(! CheckIndex(r,c))
        return;
    Set(r,c,value);
    lastPushed++;
}

std::string grid::Get(int pos)
{
    int r,c;
    r = pos/myColCount;
    c = pos%myColCount;
    if(CheckIndex(r,c))
        return at(listbox::index_pair(0,r)).text(c);
    else
        throw "Out-of-range";
}

bool grid::CheckIndex( int row, int col )
{
    if( 0 > row || row >= myRowCount ||
            0 > col || col >= myColCount )
        return false;
    return true;
}

}
