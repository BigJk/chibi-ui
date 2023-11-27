//
// Font Loader
//
const files = {
    'Ubuntu': {
        file: './data/Ubuntu-Regular.ttf',
        icons: './data/typicons.ttf',
    },
    'Ubuntu Bold': {
        file: './data/Ubuntu-Bold.ttf',
        icons: './data/typicons.ttf',
    },
    'Icons': {
        file: './data/typicons.ttf',
    },
};

const fonts = {}

export function getFont(name, size) {
    if (files[name] === undefined) {
        throw new Error(`Font ${name} does not exist`);
    }

    const id = `${name}x${size}`;
    if (fonts[id] === undefined) {
        if (files[name].icons === undefined) {
            fonts[id] = imgui.addFontFromFileTTF(id, files[name].file, size);
        } else {
            fonts[id] = imgui.addFontFromFileTTFMerged(id, files[name].file, size, files[name].icons, size, 0xe900, 0xffff);
        }
    }

    return id;
}

getFont('Ubuntu', 16);