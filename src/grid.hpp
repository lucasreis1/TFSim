#pragma once
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/listbox.hpp>

namespace nana
{


/** A grid, based on the listbox */

class grid  : public listbox
{
public:
    /** Constructor
        @param[in] wd parent window
        @param[in] r  grid size on screen pixels
    */
    grid( window wd, const rectangle& r = rectangle(), int row = 0, int col = 0);

    /** Resize the grid */
    void Resize( int rows, int cols );

    /** Title the column
        @param[in] col zero-based column index
        @param[in] value the column title
    */
    void ColTitle( int col, const std::string& value );

    /** Specify column width
     @param[in] col zero-based column index
     @param[in] width in pixels
    */
    void ColWidth( int col, int width );

    /** Set cell value */
    void Set( int row, int col, const std::string& value );

    void Set(int pos, const std::string& value );

    void Push(const std::string& value);

    std::string Get(int pos);

protected:
    int myRowCount;
    int myColCount;
    int lastPushed;
    /** true if row and col are included */
    bool CheckIndex( int row, int col );


};

}
