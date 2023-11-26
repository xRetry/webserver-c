

const PINS = [[3, 9, 0], [4, 11, 2], [5, 13, 3]];
const MODE_DESCS = ['Disabled', 'Digital Input', 'Digital Output', 'Analog Input'];

let form = document.querySelector('form');
for (const [num, modes, mode] of PINS.reverse()) {
    let select = `<select name="${num}">`;
    let i = 0;
    for (const m of modes.toString(2)) {
        const sel = i === mode ? ' selected' : '';
        if (m > 0) select += `<option value="${i}"${sel}>${MODE_DESCS[i]}</option>`
        ++i;
    }

    form.insertAdjacentHTML('afterbegin', select+'</select>');
    form.insertAdjacentHTML('afterbegin', `<label>Pin ${num}</label>`);
}




