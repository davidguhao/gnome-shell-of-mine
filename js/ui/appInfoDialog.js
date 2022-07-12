// -*- mode: js; js-indent-level: 4; indent-tabs-mode: nil -*-
/* exported AppInfoDialog */

const { Clutter, GObject, Shell, St } = imports.gi;

const Config = imports.misc.config;
const Dialog = imports.ui.dialog;
const Main = imports.ui.main;
const ModalDialog = imports.ui.modalDialog;

var DialogResponse = {
    NO_THANKS: 0,
    TAKE_TOUR: 1,
};

var AppInfoDialog = GObject.registerClass(
class AppInfoDialog extends ModalDialog.ModalDialog {
    _init() {
        // super._init({ styleClass: 'welcome-dialog' });
        super._init();

        /*
        const appSystem = Shell.AppSystem.get_default();
        this._tourAppInfo = appSystem.lookup_app('org.gnome.Tour.desktop');
        */

        // this._buildLayout();
        this.addButton({
            label: _('DONE'),
            action: () => this.close(),
            key: Clutter.KEY_Escape,
        });

        this.addButton({
            label: _('COPY DESKTOP FILE PATH'),
            action: () => {
                const clipboard = St.Clipboard.get_default();
                // clipboard.set_content(St.ClipboardType.CLIPBOARD, 'text/plain', this.desktopFilePath);
                clipboard.set_text(St.ClipboardType.CLIPBOARD, this.desktopFilePath);

                this.close();
            },
        });
    }

    open() {
        /*
        if (!this._tourAppInfo)
            return false;
            */

        return super.open();
    }
    setContent(name, id, description, desktopFilePath) {
        this.name = name;
        this.id = id;
        this.desktopFilePath = desktopFilePath;

        let title = name + " (" + id + ")";
        let descriptionInContent = (description ? description + "\n\n" : "") +
            "<DESKTOP FILE PATH>\n" + desktopFilePath;

        let content = new Dialog.MessageDialogContent({ title, description: descriptionInContent });
        this.contentLayout.add_child(content);
    }

    /*
    _buildLayout() {
        const [majorVersion] = Config.PACKAGE_VERSION.split('.');
        const title = _('Welcome to GNOME %s').format(majorVersion);
        const description = _('If you want to learn your way around, check out the tour.');
        const content = new Dialog.MessageDialogContent({ title, description });

        const icon = new St.Widget({ style_class: 'welcome-dialog-image' });
        content.insert_child_at_index(icon, 0);

        this.contentLayout.add_child(content);

        this.addButton({
            label: _('No Thanks'),
            action: () => this._sendResponse(DialogResponse.NO_THANKS),
            key: Clutter.KEY_Escape,
        });
        this.addButton({
            label: _('Take Tour'),
            action: () => this._sendResponse(DialogResponse.TAKE_TOUR),
        });
    }

    _sendResponse(response) {
        if (response === DialogResponse.TAKE_TOUR) {
            this._tourAppInfo.launch(0, -1, Shell.AppLaunchGpu.APP_PREF);
            Main.overview.hide();
        }

        this.close();
    }
    */
});
