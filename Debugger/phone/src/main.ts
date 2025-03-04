import { createApp } from "vue";
import App from "./App.vue";

createApp(App).mount("#app");
import { GridLayout, GridItem } from 'grid-layout-plus'
App
  .component('GridLayout', GridLayout)
  .component('GridItem', GridItem)
