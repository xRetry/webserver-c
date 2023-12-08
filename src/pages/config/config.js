//fetch('/modes', {
//        method: 'get',
//        headers: {
//            //'Content-type': "application/json; charset=UTF-8",
//
//        },
//    })
//    .then((response) => response.json())
//    .then((json) => console.log(json));

function build_selects(json) {
    const modes = JSON.parse(json);
    const form = document.querySelector('form');

    let pins = Array.from(new Set(modes.flatMap((mode) => mode.pins))).sort();
    for (const pin of pins.reverse()) {
        form.insertAdjacentHTML('afterbegin', `<select name="${pin}"><select>`);
        form.insertAdjacentHTML('afterbegin', `<label>Pin ${pin}</label>`);
    }

    modes.sort((a, b) => b.mode_nr - a.mode_nr);
    for (const mode of modes) {
        for (const pin of mode.pins) {
            const select = document.querySelector(`select[name="${pin}"]`);
            select.insertAdjacentHTML('afterbegin', `<option value="${mode.mode_nr}">${mode.name}</option>`);
        }
    }
}

function set_active(json) {
    for (const [pin, mode] of Object.entries(JSON.parse(active))) {
        document.querySelector(`select[name="${pin}"]`).value = mode;
    }
}

build_selects(
'[{ "mode_nr": 0, "name": "Disabled", "pins": [0,1,2,3,4,5,6,7,8,9] },{ "mode_nr": 1, "name": "Digital Write", "pins": [1,3] },{ "mode_nr": 2, "name": "Digital Read", "pins": [4,5] }]'
) 

set_active(
    '{ "1": 0, "2": 0 }'
)



