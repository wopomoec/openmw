#include "alchemywindow.hpp"

#include "../mwbase/environment.hpp"
#include "../mwworld/world.hpp"
#include "../mwworld/player.hpp"
#include "../mwworld/containerstore.hpp"

#include "window_manager.hpp"

namespace
{
    std::string getIconPath(MWWorld::Ptr ptr)
    {
        std::string path = std::string("icons\\");
        path += MWWorld::Class::get(ptr).getInventoryIcon(ptr);
        int pos = path.rfind(".");
        path.erase(pos);
        path.append(".dds");
        return path;
    }
}

namespace MWGui
{
    AlchemyWindow::AlchemyWindow(WindowManager& parWindowManager)
        : WindowBase("openmw_alchemy_window_layout.xml", parWindowManager)
        , ContainerBase(0)
    {
        getWidget(mCreateButton, "CreateButton");
        getWidget(mCancelButton, "CancelButton");
        getWidget(mIngredient1, "Ingredient1");
        getWidget(mIngredient2, "Ingredient2");
        getWidget(mIngredient3, "Ingredient3");
        getWidget(mIngredient4, "Ingredient4");
        getWidget(mApparatus1, "Apparatus1");
        getWidget(mApparatus2, "Apparatus2");
        getWidget(mApparatus3, "Apparatus3");
        getWidget(mApparatus4, "Apparatus4");

        mIngredient1->eventMouseButtonClick += MyGUI::newDelegate(this, &AlchemyWindow::onIngredientSelected);
        mIngredient2->eventMouseButtonClick += MyGUI::newDelegate(this, &AlchemyWindow::onIngredientSelected);
        mIngredient3->eventMouseButtonClick += MyGUI::newDelegate(this, &AlchemyWindow::onIngredientSelected);
        mIngredient4->eventMouseButtonClick += MyGUI::newDelegate(this, &AlchemyWindow::onIngredientSelected);

        MyGUI::Widget* buttonBox = mCancelButton->getParent();
        int cancelButtonWidth = mCancelButton->getTextSize().width + 24;
        mCancelButton->setCoord(buttonBox->getWidth() - cancelButtonWidth,
                                mCancelButton->getTop(), cancelButtonWidth, mCancelButton->getHeight());
        int createButtonWidth = mCreateButton->getTextSize().width + 24;
        mCreateButton->setCoord(buttonBox->getWidth() - createButtonWidth - cancelButtonWidth - 4,
                                mCreateButton->getTop(), createButtonWidth, mCreateButton->getHeight());

        mCreateButton->eventMouseButtonClick += MyGUI::newDelegate(this, &AlchemyWindow::onCreateButtonClicked);
        mCancelButton->eventMouseButtonClick += MyGUI::newDelegate(this, &AlchemyWindow::onCancelButtonClicked);

        MyGUI::ScrollView* itemView;
        MyGUI::Widget* containerWidget;
        getWidget(containerWidget, "Items");
        getWidget(itemView, "ItemView");
        setWidgets(containerWidget, itemView);

        center();
    }

    void AlchemyWindow::onCancelButtonClicked(MyGUI::Widget* _sender)
    {
        mWindowManager.popGuiMode();
        mWindowManager.popGuiMode();
    }

    void AlchemyWindow::onCreateButtonClicked(MyGUI::Widget* _sender)
    {
    }

    void AlchemyWindow::open()
    {
        openContainer(MWBase::Environment::get().getWorld()->getPlayer().getPlayer());
        setFilter(ContainerBase::Filter_Ingredients);

        // pick the best available apparatus
        MWWorld::ContainerStore& store = MWWorld::Class::get(mContainer).getContainerStore(mContainer);

        MWWorld::Ptr bestAlbemic;
        MWWorld::Ptr bestMortarPestle;
        MWWorld::Ptr bestCalcinator;
        MWWorld::Ptr bestRetort;

        for (MWWorld::ContainerStoreIterator it(store.begin(MWWorld::ContainerStore::Type_Apparatus));
            it != store.end(); ++it)
        {
            ESMS::LiveCellRef<ESM::Apparatus, MWWorld::RefData>* ref = it->get<ESM::Apparatus>();
            if (ref->base->data.type == ESM::Apparatus::Albemic
            && (bestAlbemic.isEmpty() || ref->base->data.quality > bestAlbemic.get<ESM::Apparatus>()->base->data.quality))
                bestAlbemic = *it;
            else if (ref->base->data.type == ESM::Apparatus::MortarPestle
            && (bestMortarPestle.isEmpty() || ref->base->data.quality > bestMortarPestle.get<ESM::Apparatus>()->base->data.quality))
                bestMortarPestle = *it;
            else if (ref->base->data.type == ESM::Apparatus::Calcinator
            && (bestCalcinator.isEmpty() || ref->base->data.quality > bestCalcinator.get<ESM::Apparatus>()->base->data.quality))
                bestCalcinator = *it;
            else if (ref->base->data.type == ESM::Apparatus::Retort
            && (bestRetort.isEmpty() || ref->base->data.quality > bestRetort.get<ESM::Apparatus>()->base->data.quality))
                bestRetort = *it;
        }

        if (!bestMortarPestle.isEmpty())
        {
            mApparatus1->setUserString("ToolTipType", "ItemPtr");
            mApparatus1->setUserData(bestMortarPestle);
            mApparatus1->setImageTexture(getIconPath(bestMortarPestle));
        }
        if (!bestAlbemic.isEmpty())
        {
            mApparatus2->setUserString("ToolTipType", "ItemPtr");
            mApparatus2->setUserData(bestAlbemic);
            mApparatus2->setImageTexture(getIconPath(bestAlbemic));
        }
        if (!bestCalcinator.isEmpty())
        {
            mApparatus3->setUserString("ToolTipType", "ItemPtr");
            mApparatus3->setUserData(bestCalcinator);
            mApparatus3->setImageTexture(getIconPath(bestCalcinator));
        }
        if (!bestRetort.isEmpty())
        {
            mApparatus4->setUserString("ToolTipType", "ItemPtr");
            mApparatus4->setUserData(bestRetort);
            mApparatus4->setImageTexture(getIconPath(bestRetort));
        }
    }

    void AlchemyWindow::onIngredientSelected(MyGUI::Widget* _sender)
    {
        _sender->clearUserStrings();
        static_cast<MyGUI::ImageBox*>(_sender)->setImageTexture("");
        if (_sender->getChildCount())
            MyGUI::Gui::getInstance().destroyWidget(_sender->getChildAt(0));
        drawItems();
        update();
    }

    void AlchemyWindow::onSelectedItemImpl(MWWorld::Ptr item)
    {
        MyGUI::ImageBox* add = NULL;

        if (!mIngredient1->isUserString("ToolTipType"))
            add = mIngredient1;
        if (add == NULL  && !mIngredient2->isUserString("ToolTipType"))
            add = mIngredient2;
        if (add == NULL  && !mIngredient3->isUserString("ToolTipType"))
            add = mIngredient3;
        if (add == NULL  && !mIngredient4->isUserString("ToolTipType"))
            add = mIngredient4;

        if (add != NULL)
        {
            add->setUserString("ToolTipType", "ItemPtr");
            add->setUserData(item);
            add->setImageTexture(getIconPath(item));
            drawItems();
            update();
        }
    }

    std::vector<MWWorld::Ptr> AlchemyWindow::itemsToIgnore()
    {
        std::vector<MWWorld::Ptr> ignore;
        // don't show ingredients that are currently selected in the "available ingredients" box.
        if (mIngredient1->isUserString("ToolTipType"))
            ignore.push_back(*mIngredient1->getUserData<MWWorld::Ptr>());
        if (mIngredient2->isUserString("ToolTipType"))
            ignore.push_back(*mIngredient2->getUserData<MWWorld::Ptr>());
        if (mIngredient3->isUserString("ToolTipType"))
            ignore.push_back(*mIngredient3->getUserData<MWWorld::Ptr>());
        if (mIngredient4->isUserString("ToolTipType"))
            ignore.push_back(*mIngredient4->getUserData<MWWorld::Ptr>());

        return ignore;
    }

    void AlchemyWindow::update()
    {
        // update ingredient count labels
        for (int i=0; i<4; ++i)
        {
            MyGUI::ImageBox* ingredient;
            if (i==0)
                ingredient = mIngredient1;
            else if (i==1)
                ingredient = mIngredient2;
            else if (i==2)
                ingredient = mIngredient3;
            else if (i==3)
                ingredient = mIngredient4;

            if (!ingredient->isUserString("ToolTipType"))
                continue;

            if (ingredient->getChildCount())
                MyGUI::Gui::getInstance().destroyWidget(ingredient->getChildAt(0));

            MyGUI::TextBox* text = ingredient->createWidget<MyGUI::TextBox>("SandBrightText", MyGUI::IntCoord(0, 14, 32, 18), MyGUI::Align::Default, std::string("Label"));
            text->setTextAlign(MyGUI::Align::Right);
            text->setNeedMouseFocus(false);
            text->setTextShadow(true);
            text->setTextShadowColour(MyGUI::Colour(0,0,0));
            text->setCaption(getCountString(ingredient->getUserData<MWWorld::Ptr>()->getRefData().getCount()));
        }
    }
}
