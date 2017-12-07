function set_cookie(cname, cvalue, exdays) {
    var d = new Date();
    d.setTime(d.getTime() + (exdays*24*60*60*1000));
    var expires = "expires="+ d.toUTCString();
    document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
}

function get_cookie(cname) {
    var name = cname + "=";
    var decodedCookie = decodeURIComponent(document.cookie);
    var ca = decodedCookie.split(';');
    for(var i = 0; i <ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') {
            c = c.substring(1);
        }
        if (c.indexOf(name) == 0) {
            return c.substring(name.length, c.length);
        }
    }
    return "";
}


function select_language(language) {
    function default_lang() {
        return (navigator.language || navigator.userLanguage).startsWith("ru") ? "ru" : "en"
    }

    if (language === undefined) {
        language = get_cookie("language");
        if (language === "") {
            language= default_lang();
        }
    }

    if (language != "en" && language != "ru") {
        language = default_lang();
    }

    set_cookie("language", language, 31)
    $("#lang-select").val(language);

    $("[lang]").each(function () {
        if ($(this).attr("lang") == language)
            $(this).show();
        else
            $(this).hide();
    });
}

select_language()

