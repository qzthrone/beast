/* BEAST - Bedevilled Audio System
 * Copyright (C) 2002-2003 Tim Janik
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
#include "bsteventrollctrl.h"
#include "bstpianorollctrl.h"


#define CONTROL_TYPE(erctrl)    ((erctrl)->eroll->control_type)


/* --- prototypes --- */
static void	controller_canvas_drag		(BstEventRollController	*self,
						 BstEventRollDrag	*drag);
static void	controller_vpanel_drag		(BstEventRollController	*self,
						 BstEventRollDrag	*drag);
static void	controller_update_cursor	(BstEventRollController *self,
						 BstEventRollTool	 tool);


/* --- variables --- */
static BsePartControlSeq *clipboard_cseq = NULL;


/* --- functions --- */
void
bst_event_roll_controller_set_clipboard (BsePartControlSeq *cseq)
{
  if (clipboard_cseq)
    bse_part_control_seq_free (clipboard_cseq);
  clipboard_cseq = cseq && cseq->n_pcontrols ? bse_part_control_seq_copy_shallow (cseq) : NULL;
  if (clipboard_cseq)
    bst_piano_roll_controller_set_clipboard (NULL);
}

BsePartControlSeq*
bst_event_roll_controller_get_clipboard (void)
{
  return clipboard_cseq;
}

BstEventRollController*
bst_event_roll_controller_new (BstEventRoll *eroll)
{
  BstEventRollController *self;
  
  g_return_val_if_fail (BST_IS_EVENT_ROLL (eroll), NULL);
  
  self = g_new0 (BstEventRollController, 1);
  self->eroll = eroll;
  self->ref_count = 1;
  
  self->ref_count++;
  g_signal_connect_data (eroll, "canvas-drag",
			 G_CALLBACK (controller_canvas_drag),
			 self, (GClosureNotify) bst_event_roll_controller_unref,
			 G_CONNECT_SWAPPED);
  g_signal_connect_data (eroll, "vpanel-drag",
			 G_CALLBACK (controller_vpanel_drag),
			 self, NULL,
			 G_CONNECT_SWAPPED);
  return self;
}

BstEventRollController*
bst_event_roll_controller_ref (BstEventRollController *self)
{
  g_return_val_if_fail (self != NULL, NULL);
  g_return_val_if_fail (self->ref_count >= 1, NULL);
  
  self->ref_count++;
  
  return self;
}

void
bst_event_roll_controller_unref (BstEventRollController *self)
{
  g_return_if_fail (self != NULL);
  g_return_if_fail (self->ref_count >= 1);
  
  self->ref_count--;
  if (!self->ref_count)
    g_free (self);
}

void
bst_event_roll_controller_set_obj_tools (BstEventRollController *self,
					 BstEventRollTool        tool1,
					 BstEventRollTool        tool2,
					 BstEventRollTool        tool3)
{
  g_return_if_fail (self != NULL);
  
  self->obj_tool1 = tool1;
  self->obj_tool2 = tool2;
  self->obj_tool3 = tool3;
}

void
bst_event_roll_controller_set_bg_tools (BstEventRollController *self,
					BstEventRollTool        tool1,
					BstEventRollTool        tool2,
					BstEventRollTool        tool3)
{
  g_return_if_fail (self != NULL);
  
  self->bg_tool1 = tool1;
  self->bg_tool2 = tool2;
  self->bg_tool3 = tool3;
  controller_update_cursor (self, self->bg_tool1);
}

void
bst_event_roll_controller_clear (BstEventRollController *self)
{
  BsePartControlSeq *cseq;
  SfiProxy proxy;
  guint i;
  
  g_return_if_fail (self != NULL);
  
  proxy = self->eroll->proxy;
  cseq = bse_part_list_selected_controls (proxy, CONTROL_TYPE (self));
  bse_item_group_undo (proxy, "Clear Selection");
  for (i = 0; i < cseq->n_pcontrols; i++)
    {
      BsePartControl *pctrl = cseq->pcontrols[i];
      bse_part_delete_event (proxy, pctrl->id);
    }
  bse_item_ungroup_undo (proxy);
}

void
bst_event_roll_controller_cut (BstEventRollController *self)
{
  BsePartControlSeq *cseq;
  SfiProxy proxy;
  guint i;
  
  g_return_if_fail (self != NULL);
  
  proxy = self->eroll->proxy;
  cseq = bse_part_list_selected_controls (proxy, CONTROL_TYPE (self));
  bse_item_group_undo (proxy, "Cut Selection");
  for (i = 0; i < cseq->n_pcontrols; i++)
    {
      BsePartControl *pctrl = cseq->pcontrols[i];
      bse_part_delete_event (proxy, pctrl->id);
    }
  bst_event_roll_controller_set_clipboard (cseq);
  bse_item_ungroup_undo (proxy);
}

gboolean
bst_event_roll_controller_copy (BstEventRollController *self)
{
  BsePartControlSeq *cseq;
  SfiProxy proxy;
  
  g_return_val_if_fail (self != NULL, FALSE);
  
  proxy = self->eroll->proxy;
  cseq = bse_part_list_selected_controls (proxy, CONTROL_TYPE (self));
  bst_event_roll_controller_set_clipboard (cseq);
  return cseq && cseq->n_pcontrols;
}

void
bst_event_roll_controller_paste (BstEventRollController *self)
{
  BsePartControlSeq *cseq;
  SfiProxy proxy;
  
  g_return_if_fail (self != NULL);
  
  proxy = self->eroll->proxy;
  cseq = bst_event_roll_controller_get_clipboard ();
  if (cseq)
    {
      guint i, ptick, ctick = self->eroll->max_ticks;
      ptick = 100; // FIXME: bst_event_roll_get_paste_pos (self->eroll, &ptick);
      for (i = 0; i < cseq->n_pcontrols; i++)
	{
	  BsePartControl *pctrl = cseq->pcontrols[i];
	  ctick = MIN (ctick, pctrl->tick);
	}
      bse_item_group_undo (proxy, "Paste Clipboard");
      bse_part_deselect_controls (proxy, 0, self->eroll->max_ticks, CONTROL_TYPE (self));
      for (i = 0; i < cseq->n_pcontrols; i++)
	{
	  BsePartControl *pctrl = cseq->pcontrols[i];
	  guint id = bse_part_insert_control (proxy,
                                              pctrl->tick - ctick + ptick,
                                              pctrl->control_type,
                                              pctrl->value);
          if (id)
            bse_part_select_event (proxy, id);
	}
      bse_item_ungroup_undo (proxy);
    }
}

gboolean
bst_event_roll_controler_clipboard_full (BstEventRollController *self)
{
  BsePartControlSeq *cseq = bst_event_roll_controller_get_clipboard ();
  return cseq && cseq->n_pcontrols;
}

static void
controller_update_cursor (BstEventRollController *self,
			  BstEventRollTool        tool)
{
  switch (tool)
    {
    case BST_EVENT_ROLL_TOOL_INSERT:
      bst_event_roll_set_canvas_cursor (self->eroll, GDK_PENCIL);
      break;
    case BST_EVENT_ROLL_TOOL_RESIZE:
      bst_event_roll_set_canvas_cursor (self->eroll, GDK_SB_V_DOUBLE_ARROW);
      break;
    case BST_EVENT_ROLL_TOOL_MOVE:
      bst_event_roll_set_canvas_cursor (self->eroll, GDK_FLEUR);
      break;
    case BST_EVENT_ROLL_TOOL_DELETE:
      bst_event_roll_set_canvas_cursor (self->eroll, GDK_TARGET);
      break;
    case BST_EVENT_ROLL_TOOL_SELECT:
      bst_event_roll_set_canvas_cursor (self->eroll, GDK_CROSSHAIR);
      break;
    default:
      bst_event_roll_set_canvas_cursor (self->eroll, GXK_DEFAULT_CURSOR);
      break;
    }
}

static void
move_start (BstEventRollController *self,
	    BstEventRollDrag       *drag)
{
  SfiProxy part = self->eroll->proxy;
  if (self->obj_id)	/* got control event to move */
    {
      controller_update_cursor (self, BST_EVENT_ROLL_TOOL_MOVE);
      gxk_status_set (GXK_STATUS_WAIT, "Move Control Event", NULL);
      drag->state = BST_DRAG_CONTINUE;
      if (bse_part_is_selected_event (part, self->obj_id))
	self->sel_cseq = bse_part_control_seq_copy_shallow (bse_part_list_selected_controls (part, CONTROL_TYPE (self)));
    }
  else
    {
      gxk_status_set (GXK_STATUS_ERROR, "Move Control Event", "No target");
      drag->state = BST_DRAG_HANDLED;
    }
}

static void
move_group_motion (BstEventRollController *self,
		   BstEventRollDrag       *drag)
{
  SfiProxy part = self->eroll->proxy;
  gint i, new_tick, delta_tick;
  
  new_tick = bst_event_roll_quantize (drag->eroll, drag->current_tick);
  delta_tick = self->obj_tick;
  delta_tick -= new_tick;
  bse_item_group_undo (part, "Move Selection");
  for (i = 0; i < self->sel_cseq->n_pcontrols; i++)
    {
      BsePartControl *pctrl = self->sel_cseq->pcontrols[i];
      gint tick = pctrl->tick;
      bse_part_change_control (part, pctrl->id,
                               MAX (tick - delta_tick, 0),
                               CONTROL_TYPE (self),
                               pctrl->value);
    }
  if (drag->type == BST_DRAG_DONE)
    {
      bse_part_control_seq_free (self->sel_cseq);
      self->sel_cseq = NULL;
    }
  bse_item_ungroup_undo (part);
}

static void
move_motion (BstEventRollController *self,
	     BstEventRollDrag       *drag)
{
  SfiProxy part = self->eroll->proxy;
  gint new_tick;
  
  if (self->sel_cseq)
    {
      move_group_motion (self, drag);
      return;
    }
  
  new_tick = bst_event_roll_quantize (drag->eroll, drag->current_tick);
  if (new_tick != self->obj_tick)
    {
      if (bse_part_change_control (part, self->obj_id, new_tick, CONTROL_TYPE (self), self->obj_value) != BSE_ERROR_NONE)
        drag->state = BST_DRAG_ERROR;
      self->obj_tick = new_tick;
    }
}

static void
move_abort (BstEventRollController *self,
	    BstEventRollDrag       *drag)
{
  if (self->sel_cseq)
    {
      bse_part_control_seq_free (self->sel_cseq);
      self->sel_cseq = NULL;
    }
  gxk_status_set (GXK_STATUS_ERROR, "Move Control Event", "Lost Event");
}

static void
insert_start (BstEventRollController *self,
	      BstEventRollDrag       *drag)
{
  SfiProxy part = self->eroll->proxy;
  BseErrorType error = BSE_ERROR_INVALID_OVERLAP;
  if (!self->obj_id && drag->start_valid)
    {
      guint qtick = bst_event_roll_quantize (drag->eroll, drag->start_tick);
      self->obj_id = bse_part_insert_control (part, qtick, CONTROL_TYPE (self), drag->current_value);
      if (self->obj_id)
        {
          self->obj_tick = qtick;
          self->obj_value = drag->current_value;
          error = BSE_ERROR_NONE;
        }
      else
        error = BSE_ERROR_NO_TARGET;
    }
  else if (!self->obj_id)
    error = BSE_ERROR_NO_TARGET;
  else /* no insertion */
    self->obj_id = 0;
  bst_status_eprintf (error, "Insert Control Event");
  drag->state = BST_DRAG_HANDLED;
}

static void
resize_start (BstEventRollController *self,
	      BstEventRollDrag       *drag)
{
  if (self->obj_id)	/* got control event for resize */
    {
      controller_update_cursor (self, BST_EVENT_ROLL_TOOL_RESIZE);
      gxk_status_set (GXK_STATUS_WAIT, "Resize Control Event", NULL);
      drag->state = BST_DRAG_CONTINUE;
    }
  else
    {
      gxk_status_set (GXK_STATUS_ERROR, "Resize Control Event", "No target");
      drag->state = BST_DRAG_HANDLED;
    }
}

static void
insert_resize_start (BstEventRollController *self,
                     BstEventRollDrag       *drag)
{
  insert_start (self, drag);
  if (self->obj_id)
    resize_start (self, drag);
}

static void
resize_motion (BstEventRollController *self,
	       BstEventRollDrag       *drag)
{
  SfiProxy part = self->eroll->proxy;
  
  /* apply new control event size */
  if (drag->current_value != self->obj_value)
    {
      bse_item_group_undo (part, "Resize Control Event");
      self->obj_value = drag->current_value;
      if (bse_part_change_control (part, self->obj_id, self->obj_tick, CONTROL_TYPE (self),
                                   self->obj_value) != BSE_ERROR_NONE)
        drag->state = BST_DRAG_ERROR;
      bse_item_ungroup_undo (part);
    }
}

static void
resize_abort (BstEventRollController *self,
	      BstEventRollDrag       *drag)
{
  gxk_status_set (GXK_STATUS_ERROR, "Resize Control Event", "Lost Event");
}

static void
delete_start (BstEventRollController *self,
	      BstEventRollDrag       *drag)
{
  SfiProxy part = self->eroll->proxy;
  if (self->obj_id)	/* got control event to delete */
    {
      BseErrorType error = bse_part_delete_event (part, self->obj_id);
      bst_status_eprintf (error, "Delete Control Event");
    }
  else
    gxk_status_set (GXK_STATUS_ERROR, "Delete Control Event", "No target");
  drag->state = BST_DRAG_HANDLED;
}

static void
select_start (BstEventRollController *self,
	      BstEventRollDrag       *drag)
{
  drag->start_tick = bst_event_roll_quantize (drag->eroll, drag->start_tick);
  bst_event_roll_set_view_selection (drag->eroll, drag->start_tick, 0);
  gxk_status_set (GXK_STATUS_WAIT, "Select Region", NULL);
  drag->state = BST_DRAG_CONTINUE;
}

static void
select_motion (BstEventRollController *self,
	       BstEventRollDrag       *drag)
{
  SfiProxy part = self->eroll->proxy;
  guint start_tick = MIN (drag->start_tick, drag->current_tick);
  guint end_tick = MAX (drag->start_tick, drag->current_tick);
  
  bst_event_roll_set_view_selection (drag->eroll, start_tick, end_tick - start_tick);
  if (drag->type == BST_DRAG_DONE)
    {
      bse_part_select_controls_exclusive (part, start_tick, end_tick - start_tick, CONTROL_TYPE (self));
      bst_event_roll_set_view_selection (drag->eroll, 0, 0);
    }
}

static void
select_abort (BstEventRollController *self,
	      BstEventRollDrag       *drag)
{
  gxk_status_set (GXK_STATUS_ERROR, "Select Region", "Aborted");
  bst_event_roll_set_view_selection (drag->eroll, 0, 0);
}

#if 0
static void
generic_abort (BstEventRollController *self,
	       BstEventRollDrag       *drag)
{
  gxk_status_set (GXK_STATUS_ERROR, "Abortion", NULL);
}
#endif

typedef void (*DragFunc) (BstEventRollController *,
			  BstEventRollDrag       *);

void
controller_canvas_drag (BstEventRollController *self,
			BstEventRollDrag       *drag)
{
  static struct {
    BstEventRollTool tool;
    DragFunc start, motion, abort;
  } tool_table[] = {
    { BST_EVENT_ROLL_TOOL_INSERT, insert_resize_start,	resize_motion,  resize_abort,	},
    { BST_EVENT_ROLL_TOOL_RESIZE,	resize_start,	resize_motion,	resize_abort,	},
    { BST_EVENT_ROLL_TOOL_MOVE,		move_start,	move_motion,	move_abort,	},
    { BST_EVENT_ROLL_TOOL_DELETE,	delete_start,	NULL,		NULL,		},
    { BST_EVENT_ROLL_TOOL_SELECT,	select_start,	select_motion,	select_abort,	},
  };
  guint i;
  
  // sfi_debug ("canvas drag event, tick=%d (valid=%d) value=%f", drag->current_tick, drag->current_valid, drag->current_value);

  if (drag->type == BST_DRAG_START)
    {
      BstEventRollTool tool = BST_EVENT_ROLL_TOOL_NONE;
      BsePartControlSeq *cseq;
      gint j, i = drag->start_tick;
      BsePartControl *nearest = NULL;
      gboolean retry_quantized = TRUE;

      /* setup drag data */
    retry_quantized:
      j = i;
      i -= drag->tick_width;
      j += drag->tick_width;
      i = MAX (i, 0);
      cseq = bse_part_list_controls (drag->eroll->proxy, i, j - i + 1, CONTROL_TYPE (self));
      j = SFI_MAXINT;
      for (i = 0; i < cseq->n_pcontrols; i++)
        {
          gint d = MAX (cseq->pcontrols[i]->tick, drag->start_tick) -
                   MIN (cseq->pcontrols[i]->tick, drag->start_tick);
          if (d < j)
            {
              j = d;
              nearest = cseq->pcontrols[i];
            }
        }
      if (!nearest && retry_quantized--)
        {
          i = bst_event_roll_quantize (drag->eroll, drag->start_tick);
          goto retry_quantized;
        }
      if (nearest)
	{
	  self->obj_id = nearest->id;
	  self->obj_tick = nearest->tick;
	  self->obj_value = nearest->value;
	}
      else
	{
	  self->obj_id = 0;
	  self->obj_tick = 0;
          self->obj_value = 0;
	}
      if (self->sel_cseq)
	g_warning ("leaking old drag selection (%p)", self->sel_cseq);
      self->sel_cseq = NULL;
      
      /* find drag tool */
      tool = BST_EVENT_ROLL_TOOL_NONE;
      if (self->obj_id)		/* have object */
	switch (drag->button)
	  {
	  case 1:	tool = self->obj_tool1;	break;
	  case 2:	tool = self->obj_tool2;	break;
	  case 3:	tool = self->obj_tool3;	break;
	  }
      else
	switch (drag->button)
	  {
	  case 1:	tool = self->bg_tool1;	break;
	  case 2:	tool = self->bg_tool2;	break;
	  case 3:	tool = self->bg_tool3;	break;
	  }
      for (i = 0; i < G_N_ELEMENTS (tool_table); i++)
	if (tool_table[i].tool == tool)
	  break;
      self->tool_index = i;
      if (i >= G_N_ELEMENTS (tool_table))
	return;		/* unhandled */
    }
  i = self->tool_index;
  g_return_if_fail (i < G_N_ELEMENTS (tool_table));
  switch (drag->type)
    {
    case BST_DRAG_START:
      if (tool_table[i].start)
	tool_table[i].start (self, drag);
      break;
    case BST_DRAG_MOTION:
    case BST_DRAG_DONE:
      if (tool_table[i].motion)
	tool_table[i].motion (self, drag);
      break;
    case BST_DRAG_ABORT:
      if (tool_table[i].abort)
	tool_table[i].abort (self, drag);
      break;
    }
  if (drag->type == BST_DRAG_DONE ||
      drag->type == BST_DRAG_ABORT)
    controller_update_cursor (self, self->bg_tool1);
}

void
controller_vpanel_drag (BstEventRollController *self,
                        BstEventRollDrag       *drag)
{
  // sfi_debug ("vpanel drag event, tick=%d (valid=%d) value=%f", drag->current_tick, drag->current_valid, drag->current_value);
  
  if (drag->type == BST_DRAG_START ||
      drag->type == BST_DRAG_MOTION)
    drag->state = BST_DRAG_CONTINUE;
}
