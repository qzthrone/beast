/* BEAST - Bedevilled Audio System
 * Copyright (C) 1998, 1999, 2000 Olaf Hoehmann and Tim Janik
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __BST_SAMPLE_SHELL_H__
#define __BST_SAMPLE_SHELL_H__

#include	"bstdefs.h"
#include	"bstsupershell.h"
#include	"bstparamview.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* --- Gtk+ type macros --- */
#define	BST_TYPE_SAMPLE_SHELL	         (bst_sample_shell_get_type ())
#define	BST_SAMPLE_SHELL(object)	 (GTK_CHECK_CAST ((object), BST_TYPE_SAMPLE_SHELL, BstSampleShell))
#define	BST_SAMPLE_SHELL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BST_TYPE_SAMPLE_SHELL, BstSampleShellClass))
#define	BST_IS_SAMPLE_SHELL(object)      (GTK_CHECK_TYPE ((object), BST_TYPE_SAMPLE_SHELL))
#define	BST_IS_SAMPLE_SHELL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BST_TYPE_SAMPLE_SHELL))
#define BST_SAMPLE_SHELL_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), BST_TYPE_SAMPLE_SHELL, BstSampleShellClass))


/* --- structures & typedefs --- */
typedef	struct	_BstSampleShell		BstSampleShell;
typedef	struct	_BstSampleShellClass	BstSampleShellClass;
struct _BstSampleShell
{
  BstSuperShell	parent_object;

  BstParamView   *param_view;
};
struct _BstSampleShellClass
{
  BstSuperShellClass	parent_class;

  gchar			*factories_path;
  GtkItemFactory	*pview_popup_factory;
};


/* --- prototypes --- */
GtkType		bst_sample_shell_get_type	(void);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BST_SAMPLE_SHELL_H__ */
