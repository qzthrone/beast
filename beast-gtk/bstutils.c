/* BEAST - Bedevilled Audio System
 * Copyright (C) 1998, 1999, 2000, 2001 Tim Janik and Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include        "bstutils.h"

#include        "bstmenus.h"
#include        <fcntl.h>
#include        <errno.h>
#include        <unistd.h>
#include        <string.h>


/* compile marshallers */
#include        "bstmarshal.c"


/* --- Pixmap Stock --- */
BseIcon*
bst_icon_from_stock (BstIconId _id)
{
#include "./icons/noicon.c"
#include "./icons/mouse_tool.c"
#include "./icons/palette.c"
#include "./icons/properties.c"
#include "./icons/trashsmall.c"
#include "./icons/trashcan.c"
#include "./icons/target.c"
#include "./icons/close.c"
#include "./icons/no_ilink.c"
#include "./icons/no_olink.c"
#include "./icons/pattern.c"
#include "./icons/pattern-group.c"
#include "./icons/pattern-tool.c"
#include "./icons/cdrom.c"
  static const BsePixdata pixdatas[] = {
    /* BST_ICON_NONE */
    { 0, 0, 0, NULL, },
    /* BST_ICON_NOICON */
    { NOICON_PIXDATA_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      NOICON_PIXDATA_WIDTH, NOICON_PIXDATA_HEIGHT,
      NOICON_PIXDATA_RLE_PIXEL_DATA, },
    /* BST_ICON_MOUSE_TOOL */
    { MOUSE_TOOL_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      MOUSE_TOOL_IMAGE_WIDTH, MOUSE_TOOL_IMAGE_HEIGHT,
      MOUSE_TOOL_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_PALETTE_TOOL */
    { PALETTE_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      PALETTE_IMAGE_WIDTH, PALETTE_IMAGE_HEIGHT,
      PALETTE_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_PROPERTIES */
    { PROPERTIES_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      PROPERTIES_IMAGE_WIDTH, PROPERTIES_IMAGE_HEIGHT,
      PROPERTIES_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_DELETE */
    { TRASHSMALL_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      TRASHSMALL_IMAGE_WIDTH, TRASHSMALL_IMAGE_HEIGHT,
      TRASHSMALL_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_TRASHCAN */
    { TRASHCAN_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      TRASHCAN_IMAGE_WIDTH, TRASHCAN_IMAGE_HEIGHT,
      TRASHCAN_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_TARGET */
    { TARGET_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      TARGET_IMAGE_WIDTH, TARGET_IMAGE_HEIGHT,
      TARGET_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_CLOSE */
    { CLOSE_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      CLOSE_IMAGE_WIDTH, CLOSE_IMAGE_HEIGHT,
      CLOSE_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_NO_ILINK */
    { NO_ILINK_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      NO_ILINK_IMAGE_WIDTH, NO_ILINK_IMAGE_HEIGHT,
      NO_ILINK_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_NO_OLINK */
    { NO_OLINK_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      NO_OLINK_IMAGE_WIDTH, NO_OLINK_IMAGE_HEIGHT,
      NO_OLINK_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_PATTERN */
    { PATTERN_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      PATTERN_IMAGE_WIDTH, PATTERN_IMAGE_HEIGHT,
      PATTERN_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_PATTERN_GROUP */
    { PATTERN_GROUP_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      PATTERN_GROUP_IMAGE_WIDTH, PATTERN_GROUP_IMAGE_HEIGHT,
      PATTERN_GROUP_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_PATTERN_TOOL */
    { PATTERN_TOOL_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      PATTERN_TOOL_IMAGE_WIDTH, PATTERN_TOOL_IMAGE_HEIGHT,
      PATTERN_TOOL_IMAGE_RLE_PIXEL_DATA, },
    /* BST_ICON_CDROM */
    { CDROM_IMAGE_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      CDROM_IMAGE_WIDTH, CDROM_IMAGE_HEIGHT,
      CDROM_IMAGE_RLE_PIXEL_DATA, },
  };
  static const guint n_stock_icons = sizeof (pixdatas) / sizeof (pixdatas[0]);
  static BseIcon *icons[sizeof (pixdatas) / sizeof (pixdatas[0])] = { NULL, };
  guint icon_id = _id;
  
  g_assert (n_stock_icons == BST_ICON_LAST);
  g_return_val_if_fail (icon_id < n_stock_icons, NULL);
  
  if (!icons[icon_id])
    {
      if (!pixdatas[icon_id].encoded_pix_data)
	return NULL;
      
      icons[icon_id] = bse_icon_from_pixdata (pixdatas + icon_id);
      bse_icon_ref_static (icons[icon_id]);
    }
  
  return icons[icon_id];
}


/* --- Gtk+ Utilities --- */
static gulong viewable_changed_id = 0;

void
gtk_widget_viewable_changed (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  g_signal_emit (widget, viewable_changed_id, 0);
}

static void
traverse_viewable_changed (GtkWidget *widget,
			   gpointer   data)
{
  if (GTK_IS_CONTAINER (widget))
    gtk_container_forall (GTK_CONTAINER (widget), (GtkCallback) gtk_widget_viewable_changed, NULL);
}

guint
gtk_tree_view_add_column (GtkTreeView       *tree_view,
			  gint               position,
			  GtkTreeViewColumn *column,
			  GtkCellRenderer   *cell,
			  const gchar       *attrib_name,
			  ...)
{
  guint n_cols;
  va_list var_args;

  g_return_val_if_fail (GTK_IS_TREE_VIEW (tree_view), 0);
  g_return_val_if_fail (GTK_IS_TREE_VIEW_COLUMN (column), 0);
  g_return_val_if_fail (column->tree_view == NULL, 0);
  g_return_val_if_fail (GTK_IS_CELL_RENDERER (cell), 0);

  g_object_ref (column);
  g_object_ref (cell);
  gtk_object_sink (GTK_OBJECT (column));
  gtk_object_sink (GTK_OBJECT (cell));
  gtk_tree_view_column_pack_start (column, cell, TRUE);

  va_start (var_args, attrib_name);
  while (attrib_name)
    {
      guint col = va_arg (var_args, guint);

      gtk_tree_view_column_add_attribute (column, cell, attrib_name, col);
      attrib_name = va_arg (var_args, const gchar*);
    }
  va_end (var_args);

  n_cols = gtk_tree_view_insert_column (tree_view, column, position);

  g_object_unref (column);
  g_object_unref (cell);

  return n_cols;
}

void
gtk_tree_selection_select_spath (GtkTreeSelection *tree_selection,
				 const gchar      *str_path)
{
  GtkTreePath *path;

  g_return_if_fail (GTK_IS_TREE_SELECTION (tree_selection));
  g_return_if_fail (str_path != NULL);

  path = gtk_tree_path_new_from_string (str_path);
  gtk_tree_selection_select_path (tree_selection, path);
  gtk_tree_path_free (path);
}

void
gtk_tree_selection_unselect_spath (GtkTreeSelection *tree_selection,
				   const gchar      *str_path)
{
  GtkTreePath *path;

  g_return_if_fail (GTK_IS_TREE_SELECTION (tree_selection));
  g_return_if_fail (str_path != NULL);

  path = gtk_tree_path_new_from_string (str_path);
  gtk_tree_selection_unselect_path (tree_selection, path);
  gtk_tree_path_free (path);
}

void
gtk_post_init_patch_ups (void)
{
  viewable_changed_id =
    g_signal_newv ("viewable-changed",
		   G_TYPE_FROM_CLASS (gtk_type_class (GTK_TYPE_WIDGET)),
		   G_SIGNAL_RUN_LAST,
		   g_cclosure_new (G_CALLBACK (traverse_viewable_changed), NULL, NULL),
		   NULL, NULL,
		   gtk_marshal_VOID__VOID,
		   G_TYPE_NONE, 0, NULL);
}

gboolean
gtk_widget_viewable (GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  while (widget)
    {
      if (!GTK_WIDGET_MAPPED (widget))
	return FALSE;
      widget = widget->parent;
    }
  return TRUE;
}

void
gtk_widget_showraise (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  
  gtk_widget_show (widget);
  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_raise (widget->window);
}

void
gtk_widget_make_sensitive (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  
  gtk_widget_set_sensitive (widget, TRUE);
}

void
gtk_widget_make_insensitive (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  
  gtk_widget_set_sensitive (widget, FALSE);
}

void
gtk_toplevel_hide (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  
  widget = gtk_widget_get_toplevel (widget);
  gtk_widget_hide (widget);
}

void
gtk_toplevel_activate_default (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  
  widget = gtk_widget_get_toplevel (widget);
  if (GTK_IS_WINDOW (widget))
    gtk_window_activate_default (GTK_WINDOW (widget));
}

void
gtk_file_selection_heal (GtkFileSelection *fs)
{
  GtkWidget *main_vbox;
  GtkWidget *hbox;
  GtkWidget *any;
  
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));
  
  /* button placement
   */
  gtk_container_set_border_width (GTK_CONTAINER (fs), 0);
  gtk_file_selection_hide_fileop_buttons (fs);
  gtk_widget_ref (fs->main_vbox);
  gtk_container_remove (GTK_CONTAINER (fs), fs->main_vbox);
  gtk_box_set_spacing (GTK_BOX (fs->main_vbox), 0);
  gtk_container_set_border_width (GTK_CONTAINER (fs->main_vbox), 5);
  main_vbox = gtk_widget_new (GTK_TYPE_VBOX,
			      "homogeneous", FALSE,
			      "spacing", 0,
			      "border_width", 0,
			      "parent", fs,
			      "visible", TRUE,
			      NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), fs->main_vbox, TRUE, TRUE, 0);
  gtk_widget_unref (fs->main_vbox);
  gtk_widget_hide (fs->ok_button->parent);
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "homogeneous", TRUE,
			 "spacing", 0,
			 "border_width", 5,
			 "visible", TRUE,
			 NULL);
  gtk_box_pack_end (GTK_BOX (main_vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_reparent (fs->ok_button, hbox);
  gtk_widget_reparent (fs->cancel_button, hbox);
  gtk_widget_grab_default (fs->ok_button);
  // gtk_label_set_text (GTK_LABEL (GTK_BIN (fs->ok_button)->child), "Ok");
  // gtk_label_set_text (GTK_LABEL (GTK_BIN (fs->cancel_button)->child), "Cancel");
  
  /* heal the action_area packing so we can customize children
   */
  gtk_box_set_child_packing (GTK_BOX (fs->action_area->parent),
			     fs->action_area,
			     FALSE, TRUE,
			     5, GTK_PACK_START);
  
  any = gtk_widget_new (GTK_TYPE_HSEPARATOR,
			"visible", TRUE,
			NULL);
  gtk_box_pack_end (GTK_BOX (main_vbox), any, FALSE, TRUE, 0);
  gtk_widget_grab_focus (fs->selection_entry);
}

static gint
idle_shower (GtkWidget **widget_p)
{
  GDK_THREADS_ENTER ();
  
  if (GTK_IS_WIDGET (*widget_p) && !GTK_OBJECT_DESTROYED (*widget_p))
    {
      gtk_signal_disconnect_by_func (GTK_OBJECT (*widget_p),
				     GTK_SIGNAL_FUNC (gtk_widget_destroyed),
				     widget_p);
      gtk_widget_show (*widget_p);
    }
  
  g_free (widget_p);
  
  GDK_THREADS_LEAVE ();
  
  return FALSE;
}

void
gtk_idle_show_widget (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (!GTK_OBJECT_DESTROYED (widget))
    {
      GtkWidget **widget_p = g_new (GtkWidget*, 1);

      *widget_p = widget;
      gtk_signal_connect (GTK_OBJECT (widget),
			  "destroy",
			  GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			  widget_p);
      gtk_idle_add_priority (GTK_PRIORITY_RESIZE - 1, (GtkFunction) idle_shower, widget_p);
    }
}

static gboolean
idle_unparent (gpointer data)
{
  GtkWidget *widget;

  GDK_THREADS_ENTER ();

  widget = GTK_WIDGET (data);
  if (widget->parent)
    gtk_container_remove (GTK_CONTAINER (widget->parent), widget);

  gtk_widget_unref (widget);

  GDK_THREADS_LEAVE ();

  return FALSE;
}

void
gtk_idle_unparent (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (widget->parent)
    {
      gtk_widget_ref (widget);
      gtk_idle_add_priority (GTK_PRIORITY_RESIZE - 1, idle_unparent, widget);
    }
}

void
gtk_last_event_coords (gint *x_root,
		       gint *y_root)
{
  GdkEvent *event = gtk_get_current_event ();
  gint x = 0, y = 0;

  if (event)
    {
      switch (event->type)
	{
	case GDK_MOTION_NOTIFY:		x = event->motion.x_root;	y = event->motion.y_root;	break;
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:	x = event->button.x_root;       y = event->button.y_root;       break;
	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:		x = event->crossing.x_root;	y = event->crossing.y_root;	break;
	case GDK_DRAG_ENTER:
	case GDK_DRAG_LEAVE:
	case GDK_DRAG_MOTION:
	case GDK_DRAG_STATUS:
	case GDK_DROP_START:
	case GDK_DROP_FINISHED:		x = event->dnd.x_root;		y = event->dnd.y_root;		break;
	default:											break;
	}
    }

  if (x_root)
    *x_root = x;
  if (y_root)
    *y_root = x;
}

void
gtk_last_event_widget_coords (GtkWidget *widget,
			      gint      *x,
			      gint      *y)
{
  gint tx, ty;

  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (GTK_WIDGET_REALIZED (widget));

  gtk_last_event_coords (&tx, &ty);
  gdk_window_translate (NULL, widget->window, &tx, &ty);

  if (x)
    *x = tx;
  if (y)
    *y = ty;
}

void
gtk_clist_moveto_selection (GtkCList *clist)
{
  g_return_if_fail (GTK_IS_CLIST (clist));

  if (GTK_WIDGET_DRAWABLE (clist) && clist->selection)
    {
      gint row = GPOINTER_TO_INT (clist->selection->data);
      
      if (gtk_clist_row_is_visible (clist, row) != GTK_VISIBILITY_FULL)
	gtk_clist_moveto (clist, row, -1, 0.5, 0);
    }
}

gpointer
gtk_clist_get_selection_data (GtkCList *clist,
			      guint     index)
{
  GList *list;
  gint row;

  g_return_val_if_fail (GTK_IS_CLIST (clist), NULL);

  list = g_list_nth (clist->selection, index);
  row = list ? GPOINTER_TO_INT (list->data) : -1;

  return row >= 0 ? gtk_clist_get_row_data (clist, row) : NULL;
}


/* --- Gtk+ Kennel --- */
struct _GtkKennel
{
  GtkKennelType	width_constraint : 8;
  GtkKennelType	height_constraint : 8;
  guint         resize_handler;
  GtkWidget    *cwidget;
  guint         width;
  guint         height;
  guint         ref_count;
  GSList       *children;
};
typedef struct
{
  GtkWidget     *widget;
  GtkRequisition real_requisition;
} KennelChild;

GtkKennel*
gtk_kennel_new (GtkKennelType width_constrain,
		GtkKennelType height_constrain)
{
  GtkKennel *kennel = g_new0 (GtkKennel, 1);

  kennel->ref_count = 1;
  gtk_kennel_configure (kennel, width_constrain, height_constrain);

  return kennel;
}

GtkKennel*
gtk_kennel_ref (GtkKennel *kennel)
{
  g_return_val_if_fail (kennel != NULL, NULL);
  g_return_val_if_fail (kennel->ref_count > 0, NULL);

  kennel->ref_count += 1;

  return kennel;
}

void
gtk_kennel_unref (GtkKennel *kennel)
{
  g_return_if_fail (kennel != NULL);
  g_return_if_fail (kennel->ref_count > 0);

  kennel->ref_count -= 1;
  if (!kennel->ref_count)
    {
      if (kennel->resize_handler)
	g_source_remove (kennel->resize_handler);
      if (kennel->cwidget)
	gtk_widget_unref (kennel->cwidget);
      g_free (kennel);
    }
}

void
gtk_kennel_configure (GtkKennel    *kennel,
		      GtkKennelType width_constrain,
		      GtkKennelType height_constrain)
{
  g_return_if_fail (kennel != NULL);
  g_return_if_fail (kennel->ref_count > 0);

  width_constrain = CLAMP (width_constrain, 0, GTK_KENNEL_TO_WIDGET);
  height_constrain = CLAMP (height_constrain, 0, GTK_KENNEL_TO_WIDGET);
  if (kennel->width_constraint != width_constrain ||
      kennel->height_constraint != height_constrain)
    {
      kennel->width_constraint = width_constrain;
      kennel->height_constraint = height_constrain;
      gtk_kennel_resize (kennel, kennel->width, kennel->height);
    }
}

void
gtk_kennel_resize (GtkKennel *kennel,
		   guint      width,
		   guint      height)
{
  GSList *slist;

  g_return_if_fail (kennel != NULL);
  g_return_if_fail (kennel->ref_count > 0);

  kennel->width = width;
  kennel->height = height;
  for (slist = kennel->children; slist; slist = slist->next)
    {
      KennelChild *child = slist->data;
      GtkRequisition *requisition = &child->widget->requisition;

      if ((kennel->width_constraint && requisition->width != kennel->width) ||
	  (kennel->height_constraint && requisition->height != kennel->height))
	gtk_widget_queue_resize (child->widget);
    }
}

static void kennel_widget_size_request (GtkWidget      *widget,
					GtkRequisition *requisition,
					GtkKennel      *kennel);

static gboolean
kennel_idle_sizer (gpointer data)
{
  GtkKennel *kennel = data;

  GDK_THREADS_ENTER ();
  
  kennel->resize_handler = 0;

  if (kennel->width_constraint == GTK_KENNEL_TO_MINIMUM ||
      kennel->width_constraint == GTK_KENNEL_TO_MAXIMUM ||
      kennel->height_constraint == GTK_KENNEL_TO_MINIMUM ||
      kennel->height_constraint == GTK_KENNEL_TO_MAXIMUM)
    {
      guint width = 0, height = 0;
      GSList *slist;
      
      for (slist = kennel->children; slist; slist = slist->next)
	{
	  KennelChild *child = slist->data;
	  GtkRequisition *requisition;

	  gtk_signal_handler_block_by_func (GTK_OBJECT (child->widget), G_CALLBACK (kennel_widget_size_request), kennel);
	  /* FIXME: GTKFIX - grrr, buggy GtkLabel code caches requisition */
	  if (GTK_IS_LABEL (child->widget))
	    {
	      gchar *string = g_strdup (GTK_LABEL (child->widget)->label);

	      gtk_label_set_text (GTK_LABEL (child->widget), NULL);
	      gtk_label_set_text (GTK_LABEL (child->widget), string);
	      g_free (string);
	    }
	  gtk_widget_size_request (child->widget, NULL);
	  gtk_signal_handler_unblock_by_func (GTK_OBJECT (child->widget), G_CALLBACK (kennel_widget_size_request), kennel);
	  requisition = &child->widget->requisition;
	  child->real_requisition = *requisition;
	  width = (kennel->width_constraint == GTK_KENNEL_TO_MINIMUM
		   ? MIN (width, requisition->width)
		   : MAX (width, requisition->width));
	  height = (kennel->height_constraint == GTK_KENNEL_TO_MINIMUM
		    ? MIN (height, requisition->height)
		    : MAX (height, requisition->height));
	}

      if (kennel->width_constraint != GTK_KENNEL_TO_MINIMUM &&
	  kennel->width_constraint != GTK_KENNEL_TO_MAXIMUM)
	width = kennel->width;
      if (kennel->height_constraint != GTK_KENNEL_TO_MINIMUM &&
	  kennel->height_constraint != GTK_KENNEL_TO_MAXIMUM)
	height = kennel->height;

      gtk_kennel_resize (kennel, width, height);
    }
  GDK_THREADS_LEAVE ();
  
  return FALSE;
}

static void
kennel_widget_size_request (GtkWidget      *widget,
			    GtkRequisition *requisition,
			    GtkKennel      *kennel)
{
  KennelChild *child = gtk_object_get_data (GTK_OBJECT (widget), "GtkKennelChild");
  GtkRequisition *real_requisition = &child->real_requisition;
  gboolean need_resize = FALSE;

  g_return_if_fail (g_slist_find (kennel->children, child));

  requisition->width = MAX (0, requisition->width);
  requisition->height = MAX (0, requisition->height);

  switch (kennel->width_constraint)
    {
    case GTK_KENNEL_TO_MINIMUM:
      if (requisition->width < kennel->width ||
	  (real_requisition->width == kennel->width &&
	   requisition->width != real_requisition->width))
	need_resize = TRUE;
      else
	requisition->width = kennel->width;
      break;
    case GTK_KENNEL_TO_MAXIMUM:
      if (requisition->width > kennel->width ||
	  (real_requisition->width == kennel->width &&
	   requisition->width != real_requisition->width))
	need_resize = TRUE;
      else
	requisition->width = kennel->width;
      break;
    case GTK_KENNEL_TO_USIZE:
    case GTK_KENNEL_TO_WIDGET:
      requisition->width = kennel->width;
      break;
    }
  switch (kennel->height_constraint)
    {
    case GTK_KENNEL_TO_MINIMUM:
      if (requisition->height < kennel->height ||
	  (real_requisition->height == kennel->height &&
	   requisition->height != real_requisition->height))
	need_resize = TRUE;
      else
	requisition->height = kennel->height;
      break;
    case GTK_KENNEL_TO_MAXIMUM:
      if (requisition->height > kennel->height ||
	  (real_requisition->height == kennel->height &&
	   requisition->height != real_requisition->height))
	need_resize = TRUE;
      else
	requisition->height = kennel->height;
      break;
    case GTK_KENNEL_TO_USIZE:
    case GTK_KENNEL_TO_WIDGET:
      requisition->height = kennel->height;
      break;
    }
  if (need_resize && !kennel->resize_handler)
    kennel->resize_handler = g_idle_add_full (GTK_PRIORITY_RESIZE - 1, kennel_idle_sizer, kennel, NULL);
}

void
gtk_kennel_add (GtkKennel *kennel,
		GtkWidget *widget)
{
  KennelChild *child;

  g_return_if_fail (kennel != NULL);
  g_return_if_fail (kennel->ref_count > 0);
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (GTK_OBJECT_DESTROYED (widget) == FALSE);
  g_return_if_fail (gtk_object_get_data (GTK_OBJECT (widget), "GtkKennelChild") == NULL);

  gtk_kennel_ref (kennel);
  child = g_new0 (KennelChild, 1);
  gtk_object_set_data (GTK_OBJECT (widget), "GtkKennelChild", child);
  child->widget = widget;
  kennel->children = g_slist_prepend (kennel->children, child);
  g_object_connect (widget,
		    "signal_after::size_request", kennel_widget_size_request, kennel,
		    "swapped_signal::destroy",  gtk_kennel_remove, kennel,
		    NULL);
  gtk_widget_queue_resize (widget);
}

void
gtk_kennel_remove (GtkKennel *kennel,
		   GtkWidget *widget)
{
  GSList *slist, *last = NULL;

  g_return_if_fail (kennel != NULL);
  g_return_if_fail (kennel->ref_count > 0);
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_object_get_data (GTK_OBJECT (widget), "GtkKennelChild") != NULL);

  gtk_object_set_data (GTK_OBJECT (widget), "GtkKennelChild", NULL);
  gtk_signal_disconnect_by_func (GTK_OBJECT (widget),
				 G_CALLBACK (kennel_widget_size_request),
				 kennel);
  gtk_signal_disconnect_by_func (GTK_OBJECT (widget),
				 G_CALLBACK (gtk_kennel_remove),
				 kennel);
  for (slist = kennel->children; slist; last = slist, slist = last->next)
    {
      KennelChild *child = slist->data;

      if (child->widget == widget)
	{
	  (last ? last->next : kennel->children) = slist->next;
	  g_slist_free_1 (slist);
	  g_free (child);
	  break;
	}
    }
  if (!kennel->resize_handler)
    kennel->resize_handler = g_idle_add_full (GTK_PRIORITY_RESIZE - 1, kennel_idle_sizer, kennel, NULL);
  if (!GTK_OBJECT_DESTROYED (widget))
    gtk_widget_queue_resize (widget);
  gtk_kennel_unref (kennel);
}


/* --- field mask --- */
static GQuark gmask_quark = 0;
typedef struct {
  GtkTooltips *tooltips;
  GtkWidget   *parent;
  GtkWidget   *prompt;
  GtkWidget   *aux1;
  GtkWidget   *aux2;		/* auto-expand */
  GtkWidget   *aux3;
  GtkWidget   *ahead;
  GtkWidget   *action;
  GtkWidget   *atail;
  gchar       *tip;
  guint	       column : 16;
  guint        expandable : 1;	/* expand action? */
  guint	       big : 1;		/* extend to left */
} GMask;
#define	GMASK_GET(o)	((GMask*) g_object_get_qdata (G_OBJECT (o), gmask_quark))

static void
gmask_destroy (gpointer data)
{
  GMask *gmask = data;

  if (gmask->tooltips)
    g_object_unref (gmask->tooltips);
  if (gmask->parent)
    g_object_unref (gmask->parent);
  if (gmask->prompt)
    g_object_unref (gmask->prompt);
  if (gmask->aux1)
    g_object_unref (gmask->aux1);
  if (gmask->aux2)
    g_object_unref (gmask->aux2);
  if (gmask->aux3)
    g_object_unref (gmask->aux3);
  if (gmask->ahead)
    g_object_unref (gmask->ahead);
  if (gmask->atail)
    g_object_unref (gmask->atail);
  g_free (gmask->tip);
  g_free (gmask);
}

static gpointer
gmask_form (GtkWidget *parent,
	    GtkWidget *action,
	    gboolean   expandable,
	    gboolean   big)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_TABLE (parent), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (action), NULL);

  if (!gmask_quark)
    gmask_quark = g_quark_from_static_string ("GMask");

  gmask = GMASK_GET (action);
  g_return_val_if_fail (gmask == NULL, NULL);

  gmask = g_new0 (GMask, 1);
  g_object_set_qdata_full (G_OBJECT (action), gmask_quark, gmask, gmask_destroy);
  gmask->parent = g_object_ref (parent);
  gtk_object_sink (GTK_OBJECT (parent));
  gmask->action = action;
  gmask->expandable = expandable != FALSE;
  gmask->big = big != FALSE;
  gmask->tooltips = g_object_get_data (G_OBJECT (parent), "GMask-tooltips");
  if (gmask->tooltips)
    g_object_ref (gmask->tooltips);

  return action;
}

/**
 * bst_gmask_container_create
 * @tooltips:     Tooltip widget
 * @border_width: Border width of this GUI mask
 *
 * Create the container for a new GUI field mask.
 */
GtkWidget*
bst_gmask_container_create (gpointer tooltips,
			    guint    border_width)
{
  GtkWidget *container = gtk_widget_new (GTK_TYPE_TABLE,
					 "visible", TRUE,
					 "homogeneous", FALSE,
					 "n_columns", 2,
					 "border_width", border_width,
					 NULL);
  if (tooltips)
    {
      g_return_val_if_fail (GTK_IS_TOOLTIPS (tooltips), container);

      g_object_set_data_full (G_OBJECT (container), "GMask-tooltips", g_object_ref (tooltips), g_object_unref);
    }

  return container;
}

gpointer
bst_gmask_form (GtkWidget *gmask_container,
		GtkWidget *action,
		gboolean   expandable)
{
  return gmask_form (gmask_container, action, expandable, FALSE);
}

gpointer
bst_gmask_form_big (GtkWidget *gmask_container,
		    GtkWidget *action)
{
  return gmask_form (gmask_container, action, TRUE, TRUE);
}

void
bst_gmask_set_tip (gpointer     mask,
		   const gchar *tip_text)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);

  g_free (gmask->tip);
  gmask->tip = g_strdup (tip_text);
}

void
bst_gmask_set_prompt (gpointer mask,
		      gpointer widget)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (gmask->prompt)
    g_object_unref (gmask->prompt);
  gmask->prompt = g_object_ref (widget);
  gtk_object_sink (GTK_OBJECT (widget));
}

void
bst_gmask_set_aux1 (gpointer mask,
		    gpointer widget)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (gmask->aux1)
    g_object_unref (gmask->aux1);
  gmask->aux1 = g_object_ref (widget);
  gtk_object_sink (GTK_OBJECT (widget));
}

void
bst_gmask_set_aux2 (gpointer mask,
		    gpointer widget)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (gmask->aux2)
    g_object_unref (gmask->aux2);
  gmask->aux2 = g_object_ref (widget);
  gtk_object_sink (GTK_OBJECT (widget));
}

void
bst_gmask_set_aux3 (gpointer mask,
		    gpointer widget)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (gmask->aux3)
    g_object_unref (gmask->aux3);
  gmask->aux3 = g_object_ref (widget);
  gtk_object_sink (GTK_OBJECT (widget));
}

void
bst_gmask_set_ahead (gpointer mask,
		     gpointer widget)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (gmask->ahead)
    g_object_unref (gmask->ahead);
  gmask->ahead = g_object_ref (widget);
  gtk_object_sink (GTK_OBJECT (widget));
}

void
bst_gmask_set_atail (gpointer mask,
		     gpointer widget)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (gmask->atail)
    g_object_unref (gmask->atail);
  gmask->atail = g_object_ref (widget);
  gtk_object_sink (GTK_OBJECT (widget));
}

void
bst_gmask_set_column (gpointer mask,
		      guint    column)
{
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);

  gmask->column = column;
}

GtkWidget*
bst_gmask_get_prompt (gpointer mask)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_WIDGET (mask), NULL);
  gmask = GMASK_GET (mask);
  g_return_val_if_fail (gmask != NULL, NULL);

  return gmask->prompt;
}

GtkWidget*
bst_gmask_get_aux1 (gpointer mask)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_WIDGET (mask), NULL);
  gmask = GMASK_GET (mask);
  g_return_val_if_fail (gmask != NULL, NULL);

  return gmask->aux1;
}

GtkWidget*
bst_gmask_get_aux2 (gpointer mask)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_WIDGET (mask), NULL);
  gmask = GMASK_GET (mask);
  g_return_val_if_fail (gmask != NULL, NULL);

  return gmask->aux2;
}

GtkWidget*
bst_gmask_get_aux3 (gpointer mask)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_WIDGET (mask), NULL);
  gmask = GMASK_GET (mask);
  g_return_val_if_fail (gmask != NULL, NULL);

  return gmask->aux3;
}

GtkWidget*
bst_gmask_get_ahead (gpointer mask)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_WIDGET (mask), NULL);
  gmask = GMASK_GET (mask);
  g_return_val_if_fail (gmask != NULL, NULL);

  return gmask->ahead;
}

GtkWidget*
bst_gmask_get_action (gpointer mask)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_WIDGET (mask), NULL);
  gmask = GMASK_GET (mask);
  g_return_val_if_fail (gmask != NULL, NULL);

  return gmask->action;
}

GtkWidget*
bst_gmask_get_atail (gpointer mask)
{
  GMask *gmask;

  g_return_val_if_fail (GTK_IS_WIDGET (mask), NULL);
  gmask = GMASK_GET (mask);
  g_return_val_if_fail (gmask != NULL, NULL);

  return gmask->atail;
}

void
bst_gmask_foreach (gpointer mask,
		   gpointer func,
		   gpointer data)
{
  GMask *gmask;
  GtkCallback callback = func;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);
  g_return_if_fail (func != NULL);

  if (gmask->prompt)
    callback (gmask->prompt, data);
  if (gmask->aux1)
    callback (gmask->aux1, data);
  if (gmask->aux2)
    callback (gmask->aux2, data);
  if (gmask->aux3)
    callback (gmask->aux3, data);
  if (gmask->ahead)
    callback (gmask->ahead, data);
  if (gmask->atail)
    callback (gmask->atail, data);
  if (gmask->action)
    callback (gmask->action, data);
}

static GtkWidget*
get_toplevel_and_set_tip (GtkWidget   *widget,
			  GtkTooltips *tooltips,
			  const gchar *tip)
{
  GtkWidget *last;

  if (!widget)
    return NULL;
  else if (!tooltips || !tip)
    return gtk_widget_get_toplevel (widget);
  do
    {
      if (!GTK_WIDGET_NO_WINDOW (widget))
	{
	  gtk_tooltips_set_tip (tooltips, widget, tip, NULL);
	  return gtk_widget_get_toplevel (widget);
	}
      last = widget;
      widget = last->parent;
    }
  while (widget);
  /* need to create a tooltips sensitive parent */
  widget = gtk_widget_new (GTK_TYPE_EVENT_BOX,
			   "visible", TRUE,
			   "child", last,
			   NULL);
  gtk_tooltips_set_tip (tooltips, widget, tip, NULL);
  return widget;
}

static guint
table_max_bottom_row (GtkTable *table,
		      guint     min_col,
		      guint	max_col)
{
  guint max_bottom = 0;
  GList *list;

  for (list = table->children; list; list = list->next)
    {
      GtkTableChild *child = list->data;

      if (child->left_attach >= min_col && child->right_attach <= max_col)
	max_bottom = MAX (max_bottom, child->bottom_attach);
    }
  return max_bottom;
}

/* GUI mask layout:
 * row: |Prompt|Aux1| Aux2 |Aux3| PreAction#Action#PostAction|
 * expandable: expand Action to left, up to Aux3
 * big row: expand Action to left as far as possible (if Prompt/Aux? are omitted)
 * Aux2 expands automatically
 */
void
bst_gmask_pack (gpointer mask)
{
  GtkWidget *prompt, *aux1, *aux2, *aux3, *ahead, *action, *atail;
  GtkTable *table;
  gboolean dummy_aux2 = FALSE;
  guint row, n, c;
  GMask *gmask;

  g_return_if_fail (GTK_IS_WIDGET (mask));
  gmask = GMASK_GET (mask);
  g_return_if_fail (gmask != NULL);

  /* retrive children and set tips */
  prompt = get_toplevel_and_set_tip (gmask->prompt, gmask->tooltips, gmask->tip);
  aux1 = get_toplevel_and_set_tip (gmask->aux1, gmask->tooltips, gmask->tip);
  aux2 = get_toplevel_and_set_tip (gmask->aux2, gmask->tooltips, gmask->tip);
  aux3 = get_toplevel_and_set_tip (gmask->aux3, gmask->tooltips, gmask->tip);
  ahead = get_toplevel_and_set_tip (gmask->ahead, gmask->tooltips, gmask->tip);
  action = get_toplevel_and_set_tip (gmask->action, gmask->tooltips, gmask->tip);
  atail = get_toplevel_and_set_tip (gmask->atail, gmask->tooltips, gmask->tip);

  /* pack children, options: GTK_EXPAND, GTK_SHRINK, GTK_FILL */
  table = GTK_TABLE (gmask->parent);
  c = 5 * gmask->column;
  row = table_max_bottom_row (table, c, c + 5);
  if (prompt)
    {
      gtk_table_attach (table, prompt, c, c + 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 0);
      gtk_table_set_col_spacing (table, c, 2); /* seperate prompt from rest */
    }
  c++;
  if (aux1)
    gtk_table_attach (table, aux1, c, c + 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 0);
  c++;
  if (!aux2)
    {
      gchar *dummy_name = g_strdup_printf ("GMask-dummy-aux2-%u", gmask->column);

      aux2 = g_object_get_data (G_OBJECT (table), dummy_name);

      /* need to have at least 1 (dummy) aux2-child per table to eat up expanding space
       */
      if (!aux2)
	{
	  aux2 = gtk_widget_new (GTK_TYPE_ALIGNMENT, "visible", TRUE, NULL);
	  g_object_set_data_full (G_OBJECT (table), dummy_name, g_object_ref (aux2), g_object_unref);
	}
      else
	aux2 = NULL;
      g_free (dummy_name);
      dummy_aux2 = TRUE;
    }
  if (aux2)
    {
      gtk_table_attach (table, aux2,
			c, c + 1,
			row, row + 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
      if (dummy_aux2)
	aux2 = NULL;
    }
  c++;
  if (aux3)
    gtk_table_attach (table, aux3, c, c + 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 0);
  c++;
  /* pack action with head and tail widgets closely together */
  if (ahead || atail)
    {
      action = gtk_widget_new (GTK_TYPE_HBOX,
			       "visible", TRUE,
			       "child", action,
			       NULL);
      if (ahead)
        gtk_container_add_with_properties (GTK_CONTAINER (action), ahead,
					   "position", 0,
					   "expand", FALSE,
					   NULL);
      if (atail)
	gtk_box_pack_end (GTK_BOX (action), atail, FALSE, TRUE, 0);
    }
  n = c;
  if (gmask->big && !aux3) /* extend action to the left when possible */
    {
      n--;
      if (!aux2)
	{
	  n--;
	  if (!aux1)
	    {
	      n--;
	      if (!prompt)
		n--;
	    }
	}
    }
  if (!gmask->expandable) /* align to right without expansion if desired */
    action = gtk_widget_new (GTK_TYPE_ALIGNMENT,
			     "visible", TRUE,
			     "child", action,
			     "xalign", 1.0,
			     "xscale", 0.0,
			     "yscale", 0.0,
			     NULL);
  gtk_table_attach (table, action,
		    n, c + 1, row, row + 1,
		    GTK_SHRINK | GTK_FILL | (gmask->expandable ? 0 /* GTK_EXPAND */ : 0),
		    GTK_FILL,
		    0, 0);
  gtk_table_set_col_spacing (table, c - 1, 2); /* seperate action from rest */
  c = 5 * gmask->column;
  if (c)
    gtk_table_set_col_spacing (table, c - 1, 5); /* spacing between columns */
}

gpointer
bst_gmask_quick (GtkWidget   *gmask_container,
		 guint	      column,
		 const gchar *prompt,
		 gpointer     action_widget,
		 const gchar *tip_text)
{
  gpointer mask = bst_gmask_form (gmask_container, action_widget, TRUE);
  
  if (prompt)
    bst_gmask_set_prompt (mask, g_object_new (GTK_TYPE_LABEL,
					      "visible", TRUE,
					      "label", prompt,
					      NULL));
  if (tip_text)
    bst_gmask_set_tip (mask, tip_text);
  bst_gmask_set_column (mask, column);
  bst_gmask_pack (mask);

  return mask;
}


/* --- BEAST utilities --- */
static void
style_modify_fg_as_sensitive (GtkWidget *widget)
{
  GtkRcStyle *rc_style = gtk_rc_style_new ();
  
  rc_style->color_flags[GTK_STATE_INSENSITIVE] = GTK_RC_FG;
  rc_style->fg[GTK_STATE_INSENSITIVE].red = widget->style->fg[GTK_STATE_NORMAL].red;
  rc_style->fg[GTK_STATE_INSENSITIVE].green = widget->style->fg[GTK_STATE_NORMAL].green;
  rc_style->fg[GTK_STATE_INSENSITIVE].blue = widget->style->fg[GTK_STATE_NORMAL].blue;
  gtk_widget_modify_style (widget, rc_style);
}

void
bst_widget_modify_as_title (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  
  if (!GTK_OBJECT_DESTROYED (widget))
    {
      gtk_widget_set_sensitive (widget, FALSE);
      if (GTK_WIDGET_REALIZED (widget))
	style_modify_fg_as_sensitive (widget);
      if (!gtk_signal_handler_pending_by_func (GTK_OBJECT (widget),
					       gtk_signal_lookup ("realize", GTK_TYPE_WIDGET),
					       TRUE,
					       style_modify_fg_as_sensitive,
					       NULL))
	gtk_signal_connect_after (GTK_OBJECT (widget), "realize", GTK_SIGNAL_FUNC (style_modify_fg_as_sensitive), NULL);
      if (!gtk_signal_handler_pending_by_func (GTK_OBJECT (widget),
					       gtk_signal_lookup ("realize", GTK_TYPE_WIDGET),
					       TRUE,
					       gtk_widget_make_insensitive,
					       NULL))
	gtk_signal_connect_after (GTK_OBJECT (widget), "realize", GTK_SIGNAL_FUNC (gtk_widget_make_insensitive), NULL);
    }
}

static void
style_modify_bg_as_base (GtkWidget *widget)
{
  GtkRcStyle *rc_style = gtk_rc_style_new ();
  
  rc_style->color_flags[GTK_STATE_NORMAL] = GTK_RC_BG;
  rc_style->bg[GTK_STATE_NORMAL].red = widget->style->base[GTK_STATE_NORMAL].red;
  rc_style->bg[GTK_STATE_NORMAL].green = widget->style->base[GTK_STATE_NORMAL].green;
  rc_style->bg[GTK_STATE_NORMAL].blue = widget->style->base[GTK_STATE_NORMAL].blue;
  gtk_widget_modify_style (widget, rc_style);
}

void
bst_widget_modify_bg_as_base (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  
  if (!GTK_OBJECT_DESTROYED (widget))
    {
      if (GTK_WIDGET_REALIZED (widget))
	style_modify_bg_as_base (widget);
      if (!gtk_signal_handler_pending_by_func (GTK_OBJECT (widget),
					       gtk_signal_lookup ("realize", GTK_TYPE_WIDGET),
					       TRUE,
					       style_modify_bg_as_base,
					       NULL))
	gtk_signal_connect_after (GTK_OBJECT (widget), "realize", GTK_SIGNAL_FUNC (style_modify_bg_as_base), NULL);
    }
}

GtkWidget*
bst_forest_from_bse_icon (BseIcon *bse_icon,
			  guint    icon_width,
			  guint    icon_height)
{
  BseIcon *icon;
  GtkWidget *forest;

  g_return_val_if_fail (bse_icon != NULL, NULL);
  g_return_val_if_fail (icon_width > 0, NULL);
  g_return_val_if_fail (icon_height > 0, NULL);

  icon = bse_icon_ref (bse_icon);
  forest = gtk_widget_new (GNOME_TYPE_FOREST,
			   "visible", TRUE,
			   "width_request", icon_width,
			   "height_request", icon_height,
			   "expand_forest", FALSE,
			   NULL);
  gtk_object_set_data_full (GTK_OBJECT (forest), "BseIcon", icon, (GDestroyNotify) bse_icon_unref);
  gnome_forest_put_sprite (GNOME_FOREST (forest), 1,
			   (icon->bytes_per_pixel > 3
			    ? art_pixbuf_new_const_rgba
			    : art_pixbuf_new_const_rgb) (icon->pixels,
							 icon->width,
							 icon->height,
							 icon->width *
							 icon->bytes_per_pixel));
  gnome_forest_set_sprite_size (GNOME_FOREST (forest), 1, icon_width, icon_height);

  return forest;
}

GtkWidget*
bst_text_view_from (GString     *gstring,
		    const gchar *file_name,
		    const gchar *font_name,
		    const gchar *font_fallback) /* FIXME: should go into misc.c or utils.c */
{
  GtkWidget *hbox, *text, *sb;
  
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "homogeneous", FALSE,
			 "spacing", 0,
			 "border_width", 5,
			 NULL);
  sb = gtk_vscrollbar_new (NULL);
  gtk_widget_show (sb);
  gtk_box_pack_end (GTK_BOX (hbox), sb, FALSE, TRUE, 0);
  text = gtk_widget_new (GTK_TYPE_TEXT,
			 "visible", TRUE,
			 "vadjustment", GTK_RANGE (sb)->adjustment,
			 "editable", FALSE,
			 "word_wrap", TRUE,
			 "line_wrap", FALSE,
			 "width_request", 500,
			 "height_request", 500,
			 "parent", hbox,
			 NULL);
  
#if !GTK_CHECK_VERSION (1, 3, 1)
  font = font_name ? gdk_font_load (font_name) : NULL;
  if (!font && font_fallback)
    font = gdk_font_load (font_fallback);
  if (font)
    {
      GtkRcStyle *rc_style = gtk_rc_style_new();
      
      gdk_font_unref (font);
      g_free (rc_style->font_name);
      rc_style->font_name = g_strdup (font_name);
      
      gtk_widget_modify_style (text, rc_style);
    }
#endif
  
  if (gstring)
    gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, gstring->str, gstring->len);
  
  if (file_name)
    {
      gint fd;
      
      fd = open (file_name, O_RDONLY, 0);
      if (fd >= 0)
	{
	  gchar buffer[512];
	  guint n;
	  
	  do
	    {
	      do
		n = read (fd, buffer, 512);
	      while (n < 0 && errno == EINTR); /* don't mind signals */
	      
	      gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, buffer, n);
	    }
	  while (n > 0);
	  close (fd);
	  
	  if (n < 0)
	    fd = -1;
	}
      if (fd < 0)
	{
	  gchar *error;
	  
	  error = g_strconcat ("Failed to load \"", file_name, "\":\n", g_strerror (errno), NULL);
	  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, error, strlen (error));
	  g_free (error);
	}
    }
  
  return hbox;
}

static void
style_modify_base_as_bg (GtkWidget *widget)
{
  GtkRcStyle *rc_style = gtk_rc_style_new ();
  
  rc_style->color_flags[GTK_STATE_NORMAL] = GTK_RC_BASE;
  rc_style->base[GTK_STATE_NORMAL].red = widget->style->bg[GTK_STATE_NORMAL].red;
  rc_style->base[GTK_STATE_NORMAL].green = widget->style->bg[GTK_STATE_NORMAL].green;
  rc_style->base[GTK_STATE_NORMAL].blue = widget->style->bg[GTK_STATE_NORMAL].blue;
  gtk_widget_modify_style (widget, rc_style);
}

static void
tweak_text_resize (GtkText *text)
{
  GtkAllocation *allocation = &GTK_WIDGET (text)->allocation;
  
  if (text->text_area)
    gdk_window_move_resize (text->text_area,
			    0, 0,
			    allocation->width,
			    allocation->height);
  gtk_adjustment_set_value (text->vadj, 0);
}

GtkWidget*
bst_wrap_text_create (const gchar *string,
		      gboolean     double_newlines,
		      gpointer     user_data)
{
  GtkWidget *text;
  
  text = g_object_connect (gtk_widget_new (GTK_TYPE_TEXT,
					   "visible", TRUE,
					   "editable", FALSE,
					   "word_wrap", TRUE,
					   "line_wrap", TRUE,
					   "can_focus", FALSE,
					   NULL),
			   "signal_after::realize", style_modify_base_as_bg, NULL, // FIXME
			   "signal_after::realize", tweak_text_resize, NULL, // FIXME
			   "signal_after::size_allocate", tweak_text_resize, NULL, // FIXME
			   NULL);
  bst_wrap_text_set (text, string, double_newlines, user_data);
  
  return text;
}

void
bst_wrap_text_set (GtkWidget   *text,
		   const gchar *string,
		   gboolean     double_newlines,
		   gpointer     user_data)
{
  g_return_if_fail (GTK_IS_TEXT (text));
  
  gtk_editable_delete_text (GTK_EDITABLE (text), 0, -1);
  if (string)
    {
      GString *gstring = g_string_new (string);
      
      if (double_newlines)
	{
	  gint i;
	  
	  for (i = 0; i < gstring->len; i++)
	    if (gstring->str[i] == '\n')
	      g_string_insert_c (gstring, i++, '\n');
	}
      gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, gstring->str, gstring->len);
      g_string_free (gstring, TRUE);
    }
  gtk_object_set_user_data (GTK_OBJECT (text), user_data);
  gtk_adjustment_set_value (GTK_TEXT (text)->vadj, 0);
}

static void
style_modify_bg_as_black (GtkWidget *widget)
{
  GtkRcStyle *rc_style = gtk_rc_style_new ();
  guint i;

  for ( i = 0; i < 5; i++)
    {
      rc_style->color_flags[i] = GTK_RC_BG;
      rc_style->bg[i].red = 0;
      rc_style->bg[i].green = 0;
      rc_style->bg[i].blue = 0;
    }
  gtk_widget_modify_style (widget, rc_style);
}

GtkWidget*
bst_drag_window_from_icon (BseIcon *icon)
{
  GtkWidget *drag_window, *forest;
  GdkBitmap *mask;
  guint8 *bitmap_data;
  gint width, height;

  g_return_val_if_fail (icon != NULL, NULL);

  /* pattern icon window
   */
  forest = bst_forest_from_bse_icon (icon,
				     BST_DRAG_ICON_WIDTH,
				     BST_DRAG_ICON_HEIGHT);
  gtk_signal_connect_after (GTK_OBJECT (forest), "realize", G_CALLBACK (style_modify_bg_as_black), NULL);
  drag_window = gtk_widget_new (GTK_TYPE_WINDOW,
				"type", GTK_WINDOW_POPUP,
				"child", forest,
				NULL);
  gtk_widget_set_app_paintable (drag_window, TRUE);
  gtk_widget_realize (drag_window);
  gdk_window_raise (drag_window->window);
  bitmap_data = gnome_forest_bitmap_data (GNOME_FOREST (forest), &width, &height);
  mask = gdk_bitmap_create_from_data (drag_window->window, bitmap_data, width, height);
  g_free (bitmap_data);
  gtk_widget_shape_combine_mask (drag_window, mask, 0, 0);
  gdk_pixmap_unref (mask);

  return drag_window;
}

guint
bst_container_get_insertion_position (GtkContainer   *container,
				      gboolean        scan_horizontally,
				      gint            xy,	/* relative to widget->window */
				      GtkWidget      *ignore_child,
				      gint           *ignore_child_position)
{
  GList *list, *children;
  GtkWidget *widget;
  guint position = 0;

  g_return_val_if_fail (GTK_IS_CONTAINER (container), -1);

  widget = GTK_WIDGET (container);

  if (ignore_child_position)
    *ignore_child_position = -1;

  children = gtk_container_children (container);
  for (list = children; list; list = list->next)
    {
      GtkWidget *child = list->data;

      if (child == ignore_child)
	{
	  if (ignore_child_position)
	    *ignore_child_position = position;
	  continue;
	}
#if 0
      if (!GTK_WIDGET_VISIBLE (child))
        continue;
#endif
      if (scan_horizontally && xy < child->allocation.x + child->allocation.width / 2)
	break;
      else if (!scan_horizontally && xy < child->allocation.y + child->allocation.height / 2)
	break;
      position++;
    }
  g_list_free (children);

  return position;
}


/* --- named children --- */
static GQuark quark_container_named_children = 0;
typedef struct {
  GData *qdata;
} NChildren;
static void
nchildren_free (gpointer data)
{
  NChildren *children = data;
  
  g_datalist_clear (&children->qdata);
  g_free (children);
}
static void
destroy_nchildren (GtkWidget *container)
{
  g_object_set_qdata (G_OBJECT (container), quark_container_named_children, NULL);
}
void
bst_container_set_named_child (GtkWidget *container,
			       GQuark     qname,
			       GtkWidget *child)
{
  NChildren *children;

  g_return_if_fail (GTK_IS_CONTAINER (container));
  g_return_if_fail (qname > 0);
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (!GTK_OBJECT_DESTROYED (child));
  if (child)
    {
      g_return_if_fail (!GTK_OBJECT_DESTROYED (container));
      g_return_if_fail (gtk_widget_is_ancestor (child, container));
    }

  if (!quark_container_named_children)
    quark_container_named_children = g_quark_from_static_string ("BstContainer-named_children");

  children = g_object_get_qdata (G_OBJECT (container), quark_container_named_children);
  if (!children)
    {
      children = g_new (NChildren, 1);
      g_datalist_init (&children->qdata);
      g_object_set_qdata_full (G_OBJECT (container), quark_container_named_children, children, nchildren_free);
      g_object_connect (container,
			"signal::destroy", destroy_nchildren, NULL,
			NULL);
    }
  g_object_ref (child);
  g_datalist_id_set_data_full (&children->qdata, qname, child, g_object_unref);
}

GtkWidget*
bst_container_get_named_child (GtkWidget *container,
			       GQuark     qname)
{
  NChildren *children;
  
  g_return_val_if_fail (GTK_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (qname > 0, NULL);

  children = quark_container_named_children ? g_object_get_qdata (G_OBJECT (container), quark_container_named_children) : NULL;
  if (children)
    {
      GtkWidget *child = g_datalist_id_get_data (&children->qdata, qname);

      if (child && !gtk_widget_is_ancestor (child, container))
	{
	  /* got removed meanwhile */
	  g_datalist_id_set_data (&children->qdata, qname, NULL);
	  child = NULL;
	}
      return child;
    }
  return NULL;
}


/* --- Canvas Utilities & Workarounds --- */
GnomeCanvasPoints*
gnome_canvas_points_new0 (guint num_points)
{
  GnomeCanvasPoints *points;
  guint i;
  
  g_return_val_if_fail (num_points > 1, NULL);
  
  points = gnome_canvas_points_new (num_points);
  for (i = 0; i < num_points; i++)
    {
      points->coords[i] = 0;
      points->coords[i + num_points] = 0;
    }
  
  return points;
}

GnomeCanvasPoints*
gnome_canvas_points_newv (guint num_points,
			  ...)
{
  GnomeCanvasPoints *points;
  guint i;
  va_list args;
  
  g_return_val_if_fail (num_points > 1, NULL);
  
  va_start (args, num_points);
  points = gnome_canvas_points_new (num_points);
  for (i = 0; i < num_points * 2; i++)
    points->coords[i] = va_arg (args, gdouble);
  va_end (args);
  
  return points;
}

GnomeCanvasItem*
gnome_canvas_typed_item_at (GnomeCanvas *canvas,
			    GtkType      item_type,
			    gdouble      world_x,
			    gdouble      world_y)
{
  GnomeCanvasItem *item;
  
  g_return_val_if_fail (GNOME_IS_CANVAS (canvas), NULL);
  
  item = gnome_canvas_get_item_at (canvas, world_x, world_y);
  while (item && !gtk_type_is_a (GTK_OBJECT_TYPE (item), item_type))
    item = item->parent;
  
  return item && gtk_type_is_a (GTK_OBJECT_TYPE (item), item_type) ? item : NULL;
}

guint
gnome_canvas_item_get_stacking (GnomeCanvasItem *item)
{
  g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), 0);
  
  if (item->parent)
    {
      GnomeCanvasGroup *parent = GNOME_CANVAS_GROUP (item->parent);
      GList *list;
      guint pos = 0;
      
      for (list = parent->item_list; list; list = list->next)
	{
	  if (list->data == item)
	    return pos;
	  pos++;
	}
    }
  
  return 0;
}

void
gnome_canvas_item_keep_between (GnomeCanvasItem *between,
				GnomeCanvasItem *item1,
				GnomeCanvasItem *item2)
{
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (between));
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (item1));
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (item2));

  if (between->parent && item1->parent && item2->parent)
    {
      if (item1->parent == between->parent && item2->parent == between->parent)
	{
	  guint n, i, z;
	  
	  n = gnome_canvas_item_get_stacking (item1);
	  i = gnome_canvas_item_get_stacking (item2);
	  z = gnome_canvas_item_get_stacking (between);
	  n = (n + i + (z > MIN (n, i))) / 2;
	  if (z < n)
	    gnome_canvas_item_raise (between, n - z);
	  else if (n < z)
	    gnome_canvas_item_lower (between, z - n);
	}
      else
	g_warning ("gnome_canvas_item_keep_between() called for non-siblings");
    }
}

void
gnome_canvas_item_keep_above (GnomeCanvasItem *above,
			      GnomeCanvasItem *item1,
			      GnomeCanvasItem *item2)
{
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (above));
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (item1));
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (item2));
  
  if (above->parent && item1->parent && item2->parent)
    {
      if (item1->parent == above->parent && item2->parent == above->parent)
	{
	  guint n, i, z;
	  
	  n = gnome_canvas_item_get_stacking (item1);
	  i = gnome_canvas_item_get_stacking (item2);
	  z = gnome_canvas_item_get_stacking (above);
	  n = MAX (n, i) + 1;
	  if (z < n)
	    gnome_canvas_item_raise (above, n - z);
	  else if (n < z)
	    gnome_canvas_item_lower (above, z - n);
	}
      else
	g_warning ("gnome_canvas_item_keep_above() called for non-siblings");
    }
}

void
gnome_canvas_FIXME_hard_update (GnomeCanvas *canvas)
{
  g_return_if_fail (GNOME_IS_CANVAS (canvas));

  /* _first_ recalc bounds of already queued items */
  gnome_canvas_update_now (canvas);

  /* just requeueing an update doesn't suffice for rect-ellipses,
   * re-translating the root-item is good enough though.
   */
  gnome_canvas_item_move (canvas->root, 0, 0);
}


/* --- Auxillary Dialogs --- */
static gpointer adialog_parent_class = NULL;

static void
bst_adialog_show (GtkWidget *widget)
{
  BstADialog *adialog = BST_ADIALOG (widget);
  GtkWindow *window = GTK_WINDOW (widget);
  
  if (adialog->default_widget)
    gtk_widget_grab_default (adialog->default_widget);
  
  if (window->focus_widget && window->default_widget &&
      GTK_WIDGET_CAN_DEFAULT (window->focus_widget) &&
      window->focus_widget != window->default_widget)
    gtk_window_set_focus (window, NULL);
  
  GTK_WIDGET_CLASS (adialog_parent_class)->show (widget);
}

static void
bst_adialog_init (BstADialog *adialog)
{
  adialog->vbox = g_object_connect (gtk_widget_new (GTK_TYPE_VBOX,
						    "visible", TRUE,
						    "border_width", 0,
						    "parent", adialog,
						    NULL),
				    "swapped_signal::destroy", bse_nullify_pointer, &adialog->vbox,
				    NULL);
  adialog->hbox = NULL;
  adialog->default_widget = NULL;
  adialog->child = NULL;
  gtk_widget_set (GTK_WIDGET (adialog),
		  "allow_shrink", FALSE,
		  "allow_grow", TRUE,
		  "events", GDK_BUTTON_PRESS_MASK,
		  NULL);
}

enum {
  ARG_NONE
};

static void
bst_adialog_force_hbox (BstADialog *adialog)
{
  if (!adialog->hbox)
    {
      adialog->hbox = g_object_connect (gtk_widget_new (GTK_TYPE_HBOX,
							"visible", TRUE,
							"border_width", 5,
							"spacing", 5,
							NULL),
					"swapped_signal::destroy", bse_nullify_pointer, &adialog->hbox,
					NULL);
      gtk_box_pack_end (GTK_BOX (adialog->vbox), adialog->hbox, FALSE, TRUE, 0);
      gtk_box_pack_end (GTK_BOX (adialog->vbox),
			gtk_widget_new (GTK_TYPE_HSEPARATOR,
					"visible", TRUE,
					NULL),
			FALSE, TRUE, 0);
    }
}

static void
bst_adialog_set_arg (GtkObject *object,
		     GtkArg    *arg,
		     guint      arg_id)
{
  // BstADialog *adialog = BST_ADIALOG (object);
  
  switch (arg_id)
    {
    default:
      break;
    }
}

void
bst_adialog_setup_choices (GtkWidget *_dialog,
			   ...)
{
  BstADialog *adialog = (gpointer) _dialog;
  gchar *name;
  va_list var_args;

  g_return_if_fail (BST_IS_ADIALOG (adialog));
  
  va_start (var_args, _dialog);
  name = va_arg (var_args, gchar*);
  while (name)
    {
      gpointer func = va_arg (var_args, gpointer);
      gpointer data = va_arg (var_args, gpointer);
      gchar *arg_name;
      guint aid;

      if (strncmp (name, "default_choice::", 16) == 0)
	aid = 16;
      else if (strncmp (name, "swapped_choice::", 16) == 0)
	aid = 16;
      else if (strncmp (name, "choice::", 8) == 0)
	aid = 8;
      else
	{
	  g_warning (G_STRLOC ": invalid choice \"%s\"", name);
	  break;
	}
      arg_name = name + aid;
      if (arg_name[0])
	{
	  GtkWidget *choice;
	  
	  bst_adialog_force_hbox (adialog);
	  
	  choice = gtk_object_get_data (GTK_OBJECT (adialog->hbox), arg_name);
	  if (!choice)
	    {
	      choice = gtk_widget_new (GTK_TYPE_BUTTON,
				       "visible", TRUE,
				       "can_default", TRUE,
				       /* "can_focus", FALSE, */
				       "label", arg_name,
				       "parent", adialog->hbox,
				       NULL);
	      gtk_object_set_data (GTK_OBJECT (adialog->hbox), arg_name, choice);
	    }
	  if (name[0] == 'd' /* default_choice */)
	    {
	      adialog->default_widget = choice;
	      gtk_widget_grab_default (adialog->default_widget);
	    }
	  if (func)
	    gtk_signal_connect_full (GTK_OBJECT (choice), "clicked",
				     func, NULL, data, NULL,
				     name[0] == 's' /* swapped_choice */,
				     FALSE);
	}
      name = va_arg (var_args, gchar*);
    }
  va_end (var_args);
}

static void
bst_adialog_class_init (BstADialogClass *class)
{
  GtkObjectClass *object_class = GTK_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
  
  adialog_parent_class = gtk_type_class (GTK_TYPE_WINDOW);
  
  object_class->set_arg = bst_adialog_set_arg;
  
  widget_class->show = bst_adialog_show;
  widget_class->delete_event = (gint (*) (GtkWidget*, GdkEventAny*)) gtk_widget_hide_on_delete;
  
  /*
    gtk_object_add_arg_type ("BstADialog::choice", GTK_TYPE_SIGNAL, GTK_ARG_WRITABLE, ARG_CHOICE);
    gtk_object_add_arg_type ("BstADialog::data_choice", GTK_TYPE_SIGNAL, GTK_ARG_WRITABLE, ARG_DATA_CHOICE);
    gtk_object_add_arg_type ("BstADialog::default_choice", GTK_TYPE_SIGNAL, GTK_ARG_WRITABLE, ARG_DEFAULT_CHOICE);
  */
}

GtkType
bst_adialog_get_type (void)
{
  static GtkType adialog_type = 0;
  
  if (!adialog_type)
    {
      GtkTypeInfo adialog_info =
      {
	"BstADialog",
	sizeof (BstADialog),
	sizeof (BstADialogClass),
	(GtkClassInitFunc) bst_adialog_class_init,
	(GtkObjectInitFunc) bst_adialog_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
	(GtkClassInitFunc) NULL,
      };
      
      adialog_type = gtk_type_unique (GTK_TYPE_WINDOW, &adialog_info);
    }
  
  return adialog_type;
}

static gboolean
destroy_on_event (GtkWidget *widget)
{
  gtk_widget_destroy (widget);
  return TRUE;
}

GtkWidget*
bst_adialog_new (GtkObject      *alive_host,
		 GtkWidget     **adialog_p,
		 GtkWidget      *child,
		 BstADialogFlags flags,
		 const gchar    *first_arg_name,
		 ...)
{
  GtkWidget *adialog;
  
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (child->parent == NULL, NULL);
  if (alive_host)
    g_return_val_if_fail (GTK_IS_OBJECT (alive_host), NULL);
  if (adialog_p)
    g_return_val_if_fail (adialog_p != NULL, NULL);
  
  adialog = g_object_connect (gtk_widget_new (BST_TYPE_ADIALOG,
					      "modal", (flags & BST_ADIALOG_MODAL) != FALSE,
					      NULL),
			      adialog_p ? "swapped_signal::destroy" : NULL, bse_nullify_pointer, adialog_p,
			      NULL);
  if (flags & BST_ADIALOG_POPUP_POS)
    gtk_widget_set (adialog, "window_position", GTK_WIN_POS_MOUSE, NULL);
  BST_ADIALOG (adialog)->child = child;
  g_object_set (GTK_WIDGET (child),
		"parent", BST_ADIALOG (adialog)->vbox,
		NULL);
  g_object_connect (GTK_WIDGET (child),
		    "swapped_signal::destroy", bse_nullify_pointer, &BST_ADIALOG (adialog)->child,
		    NULL);
  if (alive_host)
    gtk_signal_connect_object_while_alive (alive_host,
					   "destroy",
					   GTK_SIGNAL_FUNC (gtk_widget_destroy),
					   GTK_OBJECT (adialog));
  else
    gtk_quit_add_destroy (1, GTK_OBJECT (adialog));
  
  if (flags & BST_ADIALOG_FORCE_HBOX)
    bst_adialog_force_hbox (BST_ADIALOG (adialog));

  if (first_arg_name)
    {
      GObject *object = G_OBJECT (adialog);
      va_list var_args;

      g_return_val_if_fail (G_IS_OBJECT (object), NULL);

      va_start (var_args, first_arg_name);
      g_object_set_valist (object, first_arg_name, var_args);
      va_end (var_args);
    }
  
  if (flags & BST_ADIALOG_DESTROY_ON_HIDE)
    gtk_signal_connect_after (GTK_OBJECT (adialog),
			      "hide",
			      GTK_SIGNAL_FUNC (gtk_widget_destroy),
			      NULL);
  else if (flags & BST_ADIALOG_DESTROY_ON_DELETE)
    gtk_signal_connect (GTK_OBJECT (adialog),
			"delete_event",
			GTK_SIGNAL_FUNC (destroy_on_event),
			NULL);
  
  return adialog;
}

GtkWidget*
bst_adialog_get_child (GtkWidget *adialog)
{
  g_return_val_if_fail (BST_IS_ADIALOG (adialog), NULL);
  
  return BST_ADIALOG (adialog)->child;
}


/* --- Gdk utilities & workarounds --- */
#include	<gdk/gdkprivate.h>

gboolean
gdk_window_translate (GdkWindow *src_window,
		      GdkWindow *dest_window,
		      gint      *x,
		      gint      *y)
{
  gint tx = 0, ty = 0;
  gboolean success = FALSE;
#if !GTK_CHECK_VERSION (1, 3, 1)
  Window child;
  GdkWindowPrivate *src_private;
  GdkWindowPrivate *dest_private;

  src_private = src_window ? (GdkWindowPrivate*) src_window : &gdk_root_parent;
  dest_private = dest_window ? (GdkWindowPrivate*) dest_window : &gdk_root_parent;

  if (!src_private->destroyed && !dest_private->destroyed)
    success = XTranslateCoordinates (src_private->xdisplay,
				     src_private->xwindow,
				     dest_private->xwindow,
				     x ? *x : 0, y ? *y : 0, &tx, &ty,
				     &child);
#endif

  if (x)
    *x = tx;
  if (y)
    *y = ty;

  return success;
}


/* --- generated types --- */
#include "bstgentypes.c"	/* type id defs */
#include "bstenum_arrays.c"	/* enum string value arrays plus include directives */
void
bst_init_gentypes (void)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      static struct {
	gchar            *type_name;
	GType             parent;
	GType            *type_id;
	gconstpointer     pointer1;
      } builtin_info[] = {
#include "bstenum_list.c"	/* type entries */
      };
      guint i;

      for (i = 0; i < sizeof (builtin_info) / sizeof (builtin_info[0]); i++)
	{
	  GType type_id = 0;

	  if (builtin_info[i].parent == G_TYPE_ENUM)
	    type_id = g_enum_register_static (builtin_info[i].type_name, builtin_info[i].pointer1);
	  else if (builtin_info[i].parent == G_TYPE_FLAGS)
	    type_id = g_flags_register_static (builtin_info[i].type_name, builtin_info[i].pointer1);
	  else
	    g_assert_not_reached ();
	  g_assert (g_type_name (type_id) != NULL);
	  *builtin_info[i].type_id = type_id;
	}
    }
}
