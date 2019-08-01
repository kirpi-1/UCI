#include<iostream>
#include<Windows.h>
#include<conio.h>
#include<stdio.h>
#include<process.h>

#define DEFAULT_SOUNDFILE_LENGTH 128
#define METRONOME_DEFAULT_TRIAL_LENGTH 25.0

enum Conditions{SWM = 1, MWM, FWM, SWOM, MWOM, FWOM};

static std::string ConditionsNames[] =	{	
										"null",
										"slow w/ metronome, 60bpm, right foot 1",
										"med w/ metronome, 90bpm, right foot 2",
										"fast w/ metronome, 120bpm, right foot 3",
										"slow w/o metronome, 60bpm, left foot 1",
										"med w/o metronome, 90bpm, left foot 2",
										"fast w/o metronome, 120bpm, left foot 3"
										};

class Metronome{

public:
	Metronome();
	Metronome(UINT numTrials, UINT *trialList, HANDLE quitEvent = NULL, UINT startTrial = 0);
	~Metronome();

	
	UINT			curTrial(){return m_currentTrial;};
	UINT			curTrialType(){return m_trialList[m_currentTrial];};
	BOOL			playSound(){return m_playSound;};
	BOOL			playSoundToggle(){return (m_playSound = !m_playSound);};

	HANDLE			m_externalQuit;
	HANDLE			m_internalQuit;
	HANDLE			m_threadHandle;
	HANDLE			m_metronomeDone;
	
	HANDLE			start();
	BOOL			stop();
	void			reset();

	unsigned int	bpm(){return m_bpm;};
	float			time(){return m_time;};
	unsigned int	count(){return m_count;};
	unsigned int	numTrials(){return m_numTrials;};
	

	void setupMetronome(UINT bpm, float time, UINT count = 0);
	void setupExperiment(UINT nt, UINT *tl, HANDLE quitEvent = NULL, UINT startTrial = 0);
	void setSoundFile(char *fn);
	int loadMetronome();
	int loadNextMetronome();
	unsigned int	m_currentTrial;

private:
	static unsigned __stdcall play(void *obj);
	
	unsigned int	m_bpm;
	float			m_time;
	unsigned int	m_count;
	bool			m_playSound;
	wchar_t 		m_soundFile[DEFAULT_SOUNDFILE_LENGTH];

	unsigned int	m_numTrials;
	
	unsigned int	*m_trialList;
	
	
};