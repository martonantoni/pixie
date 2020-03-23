#pragma once

class cVisualizerBase
{
protected:
	cRegisteredIDList mRedrawTriggerListeners;
	cRegisteredID mRedrawID;
	cPixieWindow *mWindow=nullptr;
	int mBaseZ=100;
	virtual void Redraw()=0;
	cPixieWindow *GetWindow() const { return mWindow; }
	void RedrawNeeded();
	int GetBaseZ() const { return mBaseZ; }
public:
	void SetBaseZ(int BaseZ) { mBaseZ=BaseZ; }
};

template<class cVisualizable> class tVisualizerBase: public cVisualizerBase
{
	cVisualizable *mVisualizable=nullptr;
protected:
	cRect GetPlacement() const { return mVisualizable->GetPlacement(); }
	const cVisualizable &GetVisualizable() const { return *mVisualizable; }
	cVisualizable &GetVisualizable() { return *mVisualizable; }
public:
	virtual ~tVisualizerBase() {}
	virtual void Init(const typename cVisualizable::cInitData &) {}
	void SetVisualizable(cVisualizable *Visualizable, cPixieWindow *Window)
	{
		mWindow=Window;
		mVisualizable=Visualizable;
		mRedrawTriggerListeners.push_back(Visualizable->GetDispatcher(cVisualizable::Event_PlacementChanged)->RegisterListener([this](auto &Event) 
		{ 
			RedrawNeeded(); 
		}));
		mRedrawTriggerListeners.push_back(Visualizable->GetDispatcher(cVisualizable::Event_ParentWindowChanged)->RegisterListener([this](auto &Event) 
		{ 
			mWindow=mVisualizable->GetWindow(); RedrawNeeded(); 
		}));
		RedrawNeeded();
	}
//	virtual cPixieWindow::eHitTestResult HitTest(const cPoint &WindowRelativeCoordinates) const { return mWindow->cPixieWindow::HitTest(WindowRelativeCoordinates); }
};

template<class cVisualizable> struct cPixieVisualizerCreator
{
	typedef typename cVisualizable::cVisualizer cVisualizerBase;
	virtual std::unique_ptr<cVisualizerBase> CreateVisualizer()=0;
};

template<class cVisualizable,class T> struct tPixieVisualizerCreator: public cPixieVisualizerCreator<cVisualizable>
{
	virtual std::unique_ptr<cVisualizerBase> CreateVisualizer() override { return std::make_unique<T>(); }
	tPixieVisualizerCreator(const char *VisualizerName) { cPixieVisualizerFactory<cVisualizable>::Get().RegisterCreator(VisualizerName,this); }
};

#define REGISTER_VISUALIZER(VisualizableClass,VisualizerName,VisualizerClassName) \
	tPixieVisualizerCreator<VisualizableClass,VisualizerClassName> VisualizerClassName##Instance(VisualizerName)

template<class cVisualizable> class cPixieVisualizerFactory: public tSingleton<cPixieVisualizerFactory<cVisualizable> >
{
	typedef typename cVisualizable::cVisualizer cVisualizer;
	typedef std::map<std::string,cPixieVisualizerCreator<cVisualizable> *> cCreators;
	cCreators mCreators;
public:
	void RegisterCreator(const char *VisualizerName,cPixieVisualizerCreator<cVisualizable> *Creator);
	std::unique_ptr<cVisualizer> CreateVisualizer(cVisualizable *Visualizable, cPixieWindow *Window, const typename cVisualizable::cInitData &InitData, std::string VisualizerName);
};

template<class cVisualizable>
inline void cPixieVisualizerFactory<cVisualizable>::RegisterCreator(const char *VisualizerName,cPixieVisualizerCreator<cVisualizable> *Creator)
{
	mCreators[VisualizerName]=Creator;
}

template<class cVisualizable>
inline std::unique_ptr<typename cVisualizable::cVisualizer> cPixieVisualizerFactory<cVisualizable>::CreateVisualizer
	(cVisualizable *Visualizable, cPixieWindow *Window, const typename cVisualizable::cInitData &InitData, std::string VisualizerName)
{
	auto &Config=*InitData.mConfig;
	if(VisualizerName.empty())
		VisualizerName=Config.GetString("visualizer", { "standard" });
	if(VisualizerName=="*")
		return nullptr;
	cCreators::const_iterator i=mCreators.find(VisualizerName);
	if(i==mCreators.end())
	{
		ASSERT(false);
		return nullptr;
	}
	auto Visualizer=i->second->CreateVisualizer();
	Visualizer->SetVisualizable(Visualizable, Window);
	Visualizer->SetBaseZ(InitData.mVisualizerBaseZ);
	Visualizer->Init(InitData);
	return Visualizer;
}

#define HAS_VISUALIZER() \
	std::unique_ptr<cVisualizer> mVisualizer; \
	void InitVisualizer(const cInitData &InitData, const std::string &VisualizerName=std::string()); \
	/*virtual eHitTestResult HitTest(const cPoint &WindowRelativeCoordinates) const override { return mVisualizer->HitTest(WindowRelativeCoordinates); }*/ \
public: \
	void SetVisualizer(std::unique_ptr<cVisualizer> Visualizer); \
	void SetVisualizer(std::unique_ptr<cVisualizer> Visualizer, const cInitData &); \
protected: 

#define INIT_VISUALIZER_IMP(VisualizableClass) \
	void VisualizableClass::InitVisualizer(const VisualizableClass::cInitData &InitData, const std::string &VisualizerName) \
	{ \
		mVisualizer=cPixieVisualizerFactory<VisualizableClass>::Get().CreateVisualizer(this, GetWindow(), InitData, VisualizerName); \
	}\
	void VisualizableClass::SetVisualizer(std::unique_ptr<cVisualizer> Visualizer, const VisualizableClass::cInitData &InitData) \
	{ \
		mVisualizer=std::move(Visualizer); \
		mVisualizer->SetVisualizable(this, GetWindow()); \
		mVisualizer->SetBaseZ(InitData.mVisualizerBaseZ); \
		mVisualizer->Init(InitData); \
	}  \
	void VisualizableClass::SetVisualizer(std::unique_ptr<cVisualizer> Visualizer) \
	{ \
		SetVisualizer(std::move(Visualizer), VisualizableClass::cInitData {}); \
	} 
