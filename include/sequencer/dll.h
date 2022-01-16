#ifndef SEQUENCER_DLL_H
#define SEQUENCER_DLL_H

#ifdef SEQUENCER_LIB_EXPORT
#    define SEQUENCER_CPP_API __declspec(dllexport)
#else
#    define SEQUENCER_CPP_API __declspec(dllimport)
#endif

#endif //SEQUENCER_DLL_H
