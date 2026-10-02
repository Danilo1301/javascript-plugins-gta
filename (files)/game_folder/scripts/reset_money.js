function onLoaded() {
    CREATE_SCRIPT_ENTRY("Reset money", "reset_my_money");
}

/**
 * @param {string} data
 */
function onScriptCalled(data) {

    if (data == "reset_my_money") {

        info("Reseting money", 10000);

        const money = STORE_SCORE(0);

        info("Money before: " + money, 10000);

        ADD_SCORE(0, -money);

        const afterMoney = STORE_SCORE(0);

        info("Money after: " + afterMoney, 10000);

        ShowBottomMessage("Dinheiro resetado", 3000);
    }
}