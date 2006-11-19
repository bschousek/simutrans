#include "../../utils/cstring_t.h"
#include "../../tpl/slist_tpl.h"

#include "../bildliste_besch.h"
#include "obj_node.h"
#include "image_writer.h"
#include "obj_pak_exception.h"

#include "imagelist_writer.h"


void imagelist_writer_t::write_obj(FILE *fp, obj_node_t &parent, const slist_tpl <cstring_t> &keys)
{
    bildliste_besch_t besch;

    obj_node_t	node(this, sizeof(besch), &parent, true);

    slist_iterator_tpl<cstring_t> iter(keys);

	int count = 0;
    while(iter.next()) {
    	if((const char *)(iter.get_current())==NULL) {
    		break;
    }
	image_writer_t::instance()->write_obj(fp, node, iter.get_current());
	count ++;
  }
  if(count<keys.count()) {
	printf("WARNING: Expected %i images, but found only %i (but might be still correct)!\n",keys.count(),count );
	fflush(NULL);
  }
    besch.anzahl = count;//keys.count();

    node.write_data(fp, &besch);
    node.write(fp);
}
