/*
 * Implementation of the Microsoft Installer (msi.dll)
 *
 * Copyright 2006 Mike McCormack
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "debug.h"
#include "libmsi.h"
#include "objbase.h"
#include "objidl.h"
#include "msipriv.h"

#include "query.h"


typedef struct tagMSIALTERVIEW
{
    MSIVIEW        view;
    MSIDATABASE   *db;
    MSIVIEW       *table;
    column_info   *colinfo;
    int hold;
} MSIALTERVIEW;

static unsigned ALTER_fetch_int( MSIVIEW *view, unsigned row, unsigned col, unsigned *val )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p %d %d %p\n", av, row, col, val );

    return ERROR_FUNCTION_FAILED;
}

static unsigned ALTER_fetch_stream( MSIVIEW *view, unsigned row, unsigned col, IStream **stm)
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p %d %d %p\n", av, row, col, stm );

    return ERROR_FUNCTION_FAILED;
}

static unsigned ALTER_get_row( MSIVIEW *view, unsigned row, MSIRECORD **rec )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p %d %p\n", av, row, rec );

    return av->table->ops->get_row(av->table, row, rec);
}

static unsigned ITERATE_columns(MSIRECORD *row, void *param)
{
    (*(unsigned *)param)++;
    return ERROR_SUCCESS;
}

static BOOL check_column_exists(MSIDATABASE *db, const WCHAR *table, const WCHAR *column)
{
    MSIQUERY *view;
    MSIRECORD *rec;
    unsigned r;

    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        '`','_','C','o','l','u','m','n','s','`',' ','W','H','E','R','E',' ',
        '`','T','a','b','l','e','`','=','\'','%','s','\'',' ','A','N','D',' ',
        '`','N','a','m','e','`','=','\'','%','s','\'',0
    };

    r = MSI_OpenQuery(db, &view, query, table, column);
    if (r != ERROR_SUCCESS)
        return FALSE;

    r = MSI_ViewExecute(view, NULL);
    if (r != ERROR_SUCCESS)
        goto done;

    r = MSI_ViewFetch(view, &rec);
    if (r == ERROR_SUCCESS)
        msiobj_release(&rec->hdr);

done:
    msiobj_release(&view->hdr);
    return (r == ERROR_SUCCESS);
}

static unsigned alter_add_column(MSIALTERVIEW *av)
{
    unsigned r, colnum = 1;
    MSIQUERY *view;
    MSIVIEW *columns;

    static const WCHAR szColumns[] = {'_','C','o','l','u','m','n','s',0};
    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        '`','_','C','o','l','u','m','n','s','`',' ','W','H','E','R','E',' ',
        '`','T','a','b','l','e','`','=','\'','%','s','\'',' ','O','R','D','E','R',' ',
        'B','Y',' ','`','N','u','m','b','e','r','`',0
    };

    r = TABLE_CreateView(av->db, szColumns, &columns);
    if (r != ERROR_SUCCESS)
        return r;

    if (check_column_exists(av->db, av->colinfo->table, av->colinfo->column))
    {
        columns->ops->delete(columns);
        return ERROR_BAD_QUERY_SYNTAX;
    }

    r = MSI_OpenQuery(av->db, &view, query, av->colinfo->table, av->colinfo->column);
    if (r == ERROR_SUCCESS)
    {
        r = MSI_IterateRecords(view, NULL, ITERATE_columns, &colnum);
        msiobj_release(&view->hdr);
        if (r != ERROR_SUCCESS)
        {
            columns->ops->delete(columns);
            return r;
        }
    }
    r = columns->ops->add_column(columns, av->colinfo->table,
                                 colnum, av->colinfo->column,
                                 av->colinfo->type, (av->hold == 1));

    columns->ops->delete(columns);
    return r;
}

static unsigned ALTER_execute( MSIVIEW *view, MSIRECORD *record )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;
    unsigned ref;

    TRACE("%p %p\n", av, record);

    if (av->hold == 1)
        av->table->ops->add_ref(av->table);
    else if (av->hold == -1)
    {
        ref = av->table->ops->release(av->table);
        if (ref == 0)
            av->table = NULL;
    }

    if (av->colinfo)
        return alter_add_column(av);

    return ERROR_SUCCESS;
}

static unsigned ALTER_close( MSIVIEW *view )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p\n", av );

    return ERROR_SUCCESS;
}

static unsigned ALTER_get_dimensions( MSIVIEW *view, unsigned *rows, unsigned *cols )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p %p %p\n", av, rows, cols );

    return ERROR_FUNCTION_FAILED;
}

static unsigned ALTER_get_column_info( MSIVIEW *view, unsigned n, const WCHAR **name,
                                   unsigned *type, BOOL *temporary, const WCHAR **table_name )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p %d %p %p %p %p\n", av, n, name, type, temporary, table_name );

    return ERROR_FUNCTION_FAILED;
}

static unsigned ALTER_modify( MSIVIEW *view, MSIMODIFY eModifyMode,
                          MSIRECORD *rec, unsigned row )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p %d %p\n", av, eModifyMode, rec );

    return ERROR_FUNCTION_FAILED;
}

static unsigned ALTER_delete( MSIVIEW *view )
{
    MSIALTERVIEW *av = (MSIALTERVIEW*)view;

    TRACE("%p\n", av );
    if (av->table)
        av->table->ops->delete( av->table );
    msi_free( av );

    return ERROR_SUCCESS;
}

static unsigned ALTER_find_matching_rows( MSIVIEW *view, unsigned col,
    unsigned val, unsigned *row, MSIITERHANDLE *handle )
{
    TRACE("%p, %d, %u, %p\n", view, col, val, *handle);

    return ERROR_FUNCTION_FAILED;
}

static const MSIVIEWOPS alter_ops =
{
    ALTER_fetch_int,
    ALTER_fetch_stream,
    ALTER_get_row,
    NULL,
    NULL,
    NULL,
    ALTER_execute,
    ALTER_close,
    ALTER_get_dimensions,
    ALTER_get_column_info,
    ALTER_modify,
    ALTER_delete,
    ALTER_find_matching_rows,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

unsigned ALTER_CreateView( MSIDATABASE *db, MSIVIEW **view, const WCHAR *name, column_info *colinfo, int hold )
{
    MSIALTERVIEW *av;
    unsigned r;

    TRACE("%p %p %s %d\n", view, colinfo, debugstr_w(name), hold );

    av = msi_alloc_zero( sizeof *av );
    if( !av )
        return ERROR_FUNCTION_FAILED;

    r = TABLE_CreateView( db, name, &av->table );
    if (r != ERROR_SUCCESS)
    {
        msi_free( av );
        return r;
    }

    if (colinfo)
        colinfo->table = name;

    /* fill the structure */
    av->view.ops = &alter_ops;
    av->db = db;
    av->hold = hold;
    av->colinfo = colinfo;

    *view = &av->view;

    return ERROR_SUCCESS;
}
