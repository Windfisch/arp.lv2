#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef __cplusplus
#    include <stdbool.h>
#endif

#include <sndfile.h>

#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

typedef struct {
	LV2_URID midi_Event;
} ArpURIs;

enum {
	MIDI_IN = 0,
	MIDI_OUT = 1
};

struct Arp {
	LV2_URID_Map* map;

	LV2_Atom_Sequence const* in_port;
	LV2_Atom_Sequence* out_port;

	ArpURIs uris;

	void connect_port(uint32_t port, void* data)
	{
		switch (port) {
		case MIDI_IN:
			in_port = (const LV2_Atom_Sequence*)data;
			break;
		case MIDI_OUT:
			out_port = (LV2_Atom_Sequence*)data;
			break;
		default:
			break;
		}
	}

	Arp(LV2_URID_Map* map)
		: map(map)
		, in_port(nullptr)
		, out_port(nullptr)
	{
		uris.midi_Event = map->map(map->handle, LV2_MIDI__MidiEvent);
	}

	void run(uint32_t sample_count) {
		// Struct for a 3 byte MIDI event, used for writing notes
		struct MIDINoteEvent {
			LV2_Atom_Event event;
			uint8_t        msg[3];
		};

		// Initially self->out_port contains a Chunk with size set to capacity

		// Get the capacity
		const uint32_t out_capacity = out_port->atom.size;

		// Write an empty Sequence header to the output
		lv2_atom_sequence_clear(out_port);
		out_port->atom.type = in_port->atom.type;

		// Read incoming events
		LV2_ATOM_SEQUENCE_FOREACH(in_port, ev) {
			if (ev->body.type == uris.midi_Event) {
				const uint8_t* const msg = (const uint8_t*)(ev + 1);
				switch (lv2_midi_message_type(msg)) {
				case LV2_MIDI_MSG_NOTE_ON:
				case LV2_MIDI_MSG_NOTE_OFF:
				{
					// Forward note to output
					lv2_atom_sequence_append_event(
						out_port, out_capacity, ev);

					const uint8_t note = msg[1];
					if (note <= 127 - 7) {
						// Make a note one 5th (7 semitones) higher than input
						MIDINoteEvent fifth;
						
						// Could simply do fifth.event = *ev here instead...
						fifth.event.time.frames = ev->time.frames;  // Same time
						fifth.event.body.type   = ev->body.type;    // Same type
						fifth.event.body.size   = ev->body.size;    // Same size
						
						fifth.msg[0] = msg[0];      // Same status
						fifth.msg[1] = msg[1] + 7;  // Pitch up 7 semitones
						fifth.msg[2] = msg[2];      // Same velocity

						// Write 5th event
						lv2_atom_sequence_append_event(
							out_port, out_capacity, &fifth.event);
					}
					break;
				}
				default:
					// Forward all other MIDI events directly
					lv2_atom_sequence_append_event(
						out_port, out_capacity, ev);
					break;
				}
			}
		}
	}
};


static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
	static_cast<Arp*>(instance)->connect_port(port, data);
}

static LV2_Handle instantiate(
	const LV2_Descriptor*     descriptor,
	double rate,
	const char* path,
	const LV2_Feature* const* features
) {
	LV2_URID_Map* map = nullptr;

	// Find urid:map feature
	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_URID__map)) {
			map = (LV2_URID_Map*)features[i]->data;
		}
	}
	if (!map) {
		fprintf(stderr, "Missing feature urid:map\n");
		return nullptr;
	}

	return static_cast<LV2_Handle>(new Arp(map));
}

static void cleanup(LV2_Handle instance) {
	delete static_cast<Arp*>(instance);
}

static void run(LV2_Handle instance, uint32_t sample_count) {
	static_cast<Arp*>(instance)->run(sample_count);
}

static void const* extension_data(const char* uri) {
	return nullptr;
}

static const LV2_Descriptor descriptor = {
	"https://windfis.ch/arp.lv2",
	instantiate,
	connect_port,
	nullptr,  // activate,
	run,
	nullptr,  // deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT LV2_Descriptor const* lv2_descriptor(uint32_t index) {
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return nullptr;
	}
}
