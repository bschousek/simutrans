/*
 * convoi_frame.cc
 *
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 * filtering added by Volker Meyer
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */

#include <string.h>

#include "gui_container.h"
#include "gui_scrollpane.h"
#include "gui_convoiinfo.h"
#include "convoi_frame.h"
#include "convoi_filter_frame.h"
#include "../simvehikel.h"
#include "../simconvoi.h"
#include "../simplay.h"
#include "../simwin.h"
#include "../simworld.h"
#include "../simdepot.h"
#include "../tpl/slist_tpl.h"

#include "../besch/ware_besch.h"
#include "../bauer/warenbauer.h"
#include "../dataobj/translator.h"

 /**
 * All filter and sort settings are static, so the old settings are
 * used when the window is reopened.
 */
convoi_frame_t::sort_mode_t convoi_frame_t::sortby = convoi_frame_t::nach_name;
bool convoi_frame_t::sortreverse = false;

int convoi_frame_t::filter_flags = 0;

char convoi_frame_t::name_filter_value[64] = "";

slist_tpl<const ware_besch_t *> convoi_frame_t::waren_filter;

const char *convoi_frame_t::sort_text[SORT_MODES] = {
    "cl_btn_sort_name",
    "cl_btn_sort_income",
    "cl_btn_sort_type"
};


/**
* This function is callable by quicksort
* This function compares two convois with current sort settings.
*
* @return  1 if cnv1 > cnv2
*         -1 if cnv1 < cnv2
*          0 if cnv1 == cnv2
* @author Volker Meyer
*/
int convoi_frame_t::compare_convois(const void *p1, const void *p2)
{
    long result;

    convoihandle_t cnv1 = *(const convoihandle_t *)p1;
    convoihandle_t cnv2 = *(const convoihandle_t *)p2;

    switch (sortby) {
    default:
    case nach_name:
	result = 0;
	break;
    case nach_gewinn:
	result = cnv1->gib_jahresgewinn() - cnv2->gib_jahresgewinn();
	break;
    case nach_typ:
	{
	    vehikel_t *fahr1 = cnv1->gib_vehikel(0);
	    vehikel_t *fahr2 = cnv2->gib_vehikel(0);

	    result = fahr1->gib_typ() - fahr2->gib_typ();
	    if(result == 0) {
    		result = fahr1->gib_fracht_typ() - fahr2->gib_fracht_typ();
		if(result == 0) {
    	   	    result = fahr1->gib_basis_bild() - fahr2->gib_basis_bild();
		}
	    }
	}
	break;
    }
    /**
     * use name as an additional sort, to make sort more stable.
     */
    if(result == 0) {
	result = strcmp(cnv1->gib_name(), cnv2->gib_name());
    }
    return sortreverse ? -result : result;
}



bool convoi_frame_t::passes_filter(convoihandle_t cnv)
{
    if(!gib_filter(any_filter)) {
	return true;
    }
    vehikel_t *fahr = cnv->gib_vehikel(0);
    int i;


    if(gib_filter(name_filter) &&
       !strstr(cnv->gib_name(), name_filter_value)) {
        return false;
    }
    if(gib_filter(typ_filter)) {
	switch(fahr->gib_typ()) {
	case ding_t::automobil:
	    if(!gib_filter(lkws_filter)) {
		return false;
	    }
	    break;
	case ding_t::waggon:
	    if(!gib_filter(zuege_filter)) {
		return false;
	    }
	    break;
	case ding_t::schiff:
	    if(!gib_filter(schiffe_filter)) {
		return false;
	    }
	default:
	    break;
	}
    }
    if(gib_filter(spezial_filter)) {
        if(!(gib_filter(noroute_filter) && cnv->hat_keine_route()) &&
	   !(gib_filter(indepot_filter) && cnv->in_depot()) &&
	   !(gib_filter(noline_filter) && !cnv->has_line()) &&
	   !(gib_filter(nofpl_filter) && cnv->gib_fahrplan() == 0) &&
	   !(gib_filter(noincome_filter) && cnv->gib_jahresgewinn()/100 <= 0))
	{
	    return false;
	}
    }
    if(gib_filter(ware_filter)) {
	for(i = 0; i < cnv->gib_vehikel_anzahl(); i++) {
	    if(gib_ware_filter(cnv->gib_vehikel(i)->gib_fracht_typ()))
		break;
	}
	if(i == cnv->gib_vehikel_anzahl()) {
	    return false;;
	}
    }
    return true;
}


/**
 * Konstruktor. Erzeugt alle notwendigen Subkomponenten.
 * @author Hj. Malthaner
 */
convoi_frame_t::convoi_frame_t(spieler_t *sp, karte_t *welt) :
	gui_frame_t("cl_title", sp->kennfarbe),
	scrolly(&cont),
	sort_label(translator::translate("cl_txt_sort")),
	filter_label(translator::translate("cl_txt_filter"))
{
    owner = sp;
    this->welt = welt;
    filter_frame = NULL;

    sort_label.setze_pos(koord(3, 0));
    add_komponente(&sort_label);
    sortedby.init(button_t::roundbox, "", koord(1, 14), koord(78,14));
    sortedby.add_listener(this);
    add_komponente(&sortedby);

    sorteddir.init(button_t::roundbox, "", koord(80, 14), koord(78,14));
    sorteddir.add_listener(this);
    add_komponente(&sorteddir);

    filter_label.setze_pos(koord(164, 0));
    add_komponente(&filter_label);

    filter_on.init(button_t::roundbox, translator::translate(gib_filter(any_filter) ? "cl_btn_filter_enable" : "cl_btn_filter_disable"), koord(162, 14), koord(78,14));
    filter_on.add_listener(this);
    add_komponente(&filter_on);

    filter_details.init(button_t::roundbox, translator::translate("cl_btn_filter_settings"), koord(241, 14), koord(78,14));
    filter_details.add_listener(this);
    add_komponente(&filter_details);

    //Resize button
    vresize.setze_pos(koord(1,200));
    vresize.setze_groesse(koord(318, 6));
    vresize.set_type(gui_resizer_t::vertical_resize);

    scrolly.setze_pos(koord(1, 30));
    setze_opaque(true);
    setze_fenstergroesse(koord(320, 191+16+16));

    display_list();
    add_komponente(&scrolly);

    add_komponente(&vresize);
    vresize.add_listener(this);

    resize (koord(0,0));
}


/**
 * Destruktor.
 * @author Hj. Malthaner
 */
convoi_frame_t::~convoi_frame_t()
{
}



void convoi_frame_t::display_list(void)
{
    slist_iterator_tpl<convoihandle_t> iter (welt->gib_convoi_list());
    const int count = welt->gib_convoi_list().count();

#ifdef _MSC_VER
    convoihandle_t *a = new convoihandle_t[count];
#else
    convoihandle_t a[count];
#endif
    int n = 0;
    int ypos = 0;
    int i;

    while(iter.next()) {
	convoihandle_t cnv = iter.get_current();

	if(cnv->gib_besitzer() == owner && passes_filter(cnv)) {
	    a[n++] = cnv;
	}
    }
    qsort((void *)a, n, sizeof (convoihandle_t), compare_convois);

    sortedby.setze_text(translator::translate(sort_text[gib_sortierung()]));
    sorteddir.setze_text(translator::translate(gib_reverse() ? "cl_btn_sort_desc" : "cl_btn_sort_asc"));

    cont.remove_all();

    for (i = 0; i < n; i++) {
	gui_convoiinfo_t *cinfo = new gui_convoiinfo_t(a[i], i + 1);

	cinfo->setze_pos(koord(0, ypos));
        cinfo->setze_groesse(koord(500, 32));
	cont.add_komponente(cinfo);
        ypos += 40; // @author hsiegeln: changed from +=32 to +=40 to have more space for "serves line" info!
    }
    cont.setze_groesse(koord(500, ypos));
    scrolly.setze_groesse(koord(318, gib_fenstergroesse().y - 1 - 16 - 16 - 20));
#ifdef _MSC_VER
    delete [] a;
#endif
}



void convoi_frame_t::infowin_event(const event_t *ev)
{
    if(ev->ev_class == INFOWIN &&
       ev->ev_code == WIN_CLOSE) {
	if(filter_frame) {
	    filter_frame->infowin_event(ev);
	}
    }
    gui_frame_t::infowin_event(ev);
}

/**
 * This method is called if an action is triggered
 * @author Markus Weber
 */
bool convoi_frame_t::action_triggered(gui_komponente_t *komp)           // 28-Dec-01    Markus Weber    Added
{
    if(komp == &vresize) {
	resize (koord(vresize.get_hresize(), vresize.get_vresize()));
    } else if(komp == &filter_on) {
	setze_filter(any_filter, !gib_filter(any_filter));
	filter_on.setze_text(translator::translate(gib_filter(any_filter) ? "cl_btn_filter_enable" : "cl_btn_filter_disable"));
	display_list();
    } else if(komp == &sortedby) {
    	setze_sortierung((sort_mode_t)((gib_sortierung() + 1) % SORT_MODES));
	display_list();
    } else if(komp == &sorteddir) {
    	setze_reverse(!gib_reverse());
	display_list();
    } else if(komp == &filter_details) {
	if(filter_frame) {
	    destroy_win(filter_frame);
	}
	else {
	    filter_frame = new convoi_filter_frame_t(owner, this);
	    create_win(filter_frame, w_autodelete, -1);
	}
    }
    return true;
}


void convoi_frame_t::resize(koord size_change)                          // 28-Dec-01    Markus Weber    Added
{

    koord new_windowsize = gib_fenstergroesse() + size_change;
    int vresize_top = vresize.gib_pos().y + size_change.y;


    if (new_windowsize.y < 100 )
    {
        new_windowsize.y = 100;
        vresize_top=100-23 ;
        vresize.cancelresize();
    }

    setze_fenstergroesse (new_windowsize);

    scrolly.setze_groesse(koord(318, gib_fenstergroesse().y - 1 - 16 - 16 - 20));
    vresize.setze_pos(koord(1,  vresize_top));
}



void convoi_frame_t::zeichnen(koord pos, koord gr)
{
    filter_details.pressed = filter_frame != NULL;
    gui_frame_t::zeichnen(pos, gr);
}

void convoi_frame_t::setze_ware_filter(const ware_besch_t *ware, int mode)
{
    if(ware != warenbauer_t::nichts) {
	if(gib_ware_filter(ware)) {
	    if(mode != 1) {
		waren_filter.remove(ware);
	    }
	} else {
	    if(mode != 0) {
		waren_filter.append(ware);
	    }
	}
    }
}


void convoi_frame_t::setze_alle_ware_filter(int mode)
{
    if(mode == 0) {
	waren_filter.clear();
    }
    else {
	for(unsigned int i = 0; i<warenbauer_t::gib_waren_anzahl(); i++) {
	    setze_ware_filter(warenbauer_t::gib_info(i), mode);
	}
    }
}
