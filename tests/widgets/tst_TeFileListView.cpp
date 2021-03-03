/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

/*
 * TeFileListView
 *
 * - set/get FolderView
 * - KeyPress
 * - Selection (Provide Tac and explorer  mode)
 *     - by Command : all commands are original. so test is not necessally.
 *     - by KeyPress :
 *         - special key
 *             KeyPress Eater is set by parent view.  variation is below
 *             [Ctrl]? + [A-Z]
 *             [Ctrl,Shift,Ctrl+Shift]? + [F1-F12]
 *             [Escape,Backspace,Delete]
 *             so these KeyPress Events must not any effect to selection.
 *         - selection key
 *             - cursor
 *                 - Press the cursor (up,down,left,right) key to move the current cursor position by one item.
 *                 - If next positon is corner then it is not effect to cursor.
 *                 - these are not effect to selection for tac mode.
 *                   clear selection and select current item for explorer mode.
 *             - space
 *                 - Toggle Selection of current item and Move cursor(current item) to next item for tac mode.
 *                   if current item is last item then selection only.
 *                   if no item is there then no effect.
 *                 - no effect for explorer mode.
 *     - by Mouse :
 *         number of items : nothing or exists.
 *         mouse action    : over , press , release , click , double  click
 *         drag state      : drag start , dragging , drag end.
 *         mouse botton    : left , right , middle
 *         modifier        : ctrl , shift , ctrl + shift
 *         selection state : nothing or exists.
 *         target item state : none , bare (not current, not selected) , current , selected , current + selected
 *
 *
 *         - over the item.
 *             - highlight the pointed item.
 *             - selected state has priority.
 *         - left button
 *             - press
 *                 - for no item pointted.
 *                     - no effect. it start trigger for dragging selection.
 *                 - for pointing item if it is not selected.
 *                     - with no modifier key
 *                         - change current cursor to pressed item.
 *                         - clear selection and select pressed item.
 *                         - after action
 *                             - space key: reselect pressed item
 *                             - isolate modifier key : no effect continue after action.
 *                             - other key : deselect pressed item
 *                             - mouse press pressed index with no selection effect: continue after action.
 *                             - mouse press with other action without ClearAndSelect : cancel after action.
 *                     - with ctrl key
 *                         - change current cursor to pressed item.
 *                         - add pressed item to selection. (it does not clear current selection.)
 *                     - with shift key
 *                         - do not change current cursor position.
 *                         - clear selection and select items that is between previous item to current item.
 *                     - with ctrl+shift key
 *                         - same as shift operation. but it does not clear selection.
 *                 - for pointing item if it is selected.
 *                     - with no modifier key
 *                         this action provide by relase or drag
 *                     - with ctrl key
 *                         this action provide by relase or drag
 *                     - with shift key
 *                         - same as "target item is not selected".
 *                     - with ctrl+shift key
 *                         - same as "target item is not selected".
 *             - release
 *                 - for no item pointted.
 *                     - with no modifier
 *                         - dragging was started.
 *                             - no effect
 *                         - dragging was not started.
 *                             - clear selection.
 *                     - with shift or ctrl
 *                         - no effect
 *                 - for item pointed if it is not selected.
 *                     - no effect (Already working on the press)
 *                 - for item pointed if it is selected.
 *                     - with no modifier key
 *                         - dragging was started.
 *                             - no effect ( it provied at drag end)
 *                         - dragging was not started.
 *                             - clear selection
 *                             - select pointed item
 *                             - move cursor to pointed item.
 *                     - with ctrl key
 *                         - dragging was started.
 *                             - no effect ( it provied at drag end)
 *                         - dragging was not started.
 *                             - toggle select of pointed item and change current cursor position.
 *                     - with shift or ctrl + shift
 *                         - no effect (Already working on the press)
 *             - click (include operation of press)
 *                 - no effect
 *             - double  Click (include operation of press)
 *                 - emit activate signal with pressed item.
 *                 - if any item is note pointed then nothing to do.
 *             - drag
 *                 - start
 *                     - from brank
 *                         - start select by region
 *                             - no modifier : clear selection before start
 *                             - + ctrl      : start toggle selection
 *                             - + shift     : start append selection
 *                     - from not selected item
 *                         - do press action before start dragging. so this action is same as selected item.
 *                     - from selected item
 *                         - start drag & drop action
 *                 - dragging
 *                     - select by region
 *                         - no modifier : update selection by region
 *                         - + ctrl      : toggle selection by region
 *                         - + shift     : append selection by region
 *                     - drag & drop action
 *                 - end
 *                     - select by region
 *                         - release
 *                             - fix selection same as dragging
 *                         - press another mouse button
 *                             - fix selection same as dragging
 *                     - drag & drop action
 *                         - drag in and drop
 *                             - default clipboard action. (not affect by modifier)
 *                         - drag out and drop
 *                             - action by windows
 *                         - drop
 *                             - same as drag and drop.
 *                               but if drop target as current path ( not point to not seleted folder)
 *                                  then no effect.
 *                         - press another mouse button
 *                             no effect
 *         - right button
 *             - press
 *                 - same as left button
 *             - release
 *                 - no effect (it only work by click)
 *             - click (include operation of press)
 *                 - popup context menu.
 *             - double  Click (include operation of press)
 *                 - no effect
 *             - drag
 *                 - start
 *                     - from brank
 *                         - no effect
 *                     - from not selected item
 *                         - do press action before start dragging. so this action is same as selected item.
 *                     - from selected item
 *                         - start drag & drop action
 *                 - dragging
 *                     - drag & drop action
 *                 - end
 *                     - drag & drop action
 *                         - drag in and drop
 *                             - open drop action context menu.
 *                         - drag out and drop
 *                             - action by windows
 *                         - drop
 *                             - same as drag and drop.
 *                               but if drop target as current path ( not point to not seleted folder)
 *                                  then no effect.
 *                         - press another mouse button
 *                             no effect
 *         - middle button
 *             - no effect
 * - signal : Item activated
 * - signal : customContextMenuRequested
 * - drag & drop
 *
 */
